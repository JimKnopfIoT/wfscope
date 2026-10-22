/*
 * wfscope-icap — Root-Helfer für den ICAP-IQ-Capture des connac2x-WLAN-Chips.
 *
 * Protokoll (stdin, zeilenweise):
 *   cap <kanal> <bw> <band>   bw: 0=20 1=40 2=80 3=160 MHz, band: 0=2,4 1=5 2=6 GHz
 *   quit
 * Antwort auf stdout pro "cap": struct frame_hdr + n * (int16 I, int16 Q), WF0.
 *
 * Wiederherstellung (ICAP stoppen, Testmodus abbrechen) bei quit, stdin-EOF,
 * SIGTERM/SIGINT/SIGHUP und — über einen abgekoppelten Watchdog-Prozess —
 * auch wenn dieser Prozess hart beendet wird.
 * Nie "0" nach /dev/wmtWifi schreiben: das schaltet den Chip ab.
 */
#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <linux/wireless.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define IFNAME "wlan0"
#define HQA_MAGIC 0x18142880u
#define HQA_BUF 0x80c
#define IOCTL_TESTMODE 0x8BE8 /* SIOCIWFIRSTPRIV + 8: NDIS-OID */
#define IOCTL_QA_TOOL 0x8BF0  /* SIOCIWFIRSTPRIV + 16: HQA */
#define OID_TEST_ICAP_MODE 0xFFA0C913u
#define OID_ABORT_TEST_MODE 0xFFA0C906u
#define ICAP_NODE 0
#define ICAP_LEN 400
#define ICAP_STOP 400
#define MAX_SAMPLES 32768

struct frame_hdr {
    char magic[4];      /* "WFIQ" */
    uint16_t version;   /* 1 */
    uint16_t status;    /* 0 = ok, sonst Fehlercode */
    uint32_t channel;
    uint32_t bw;
    uint32_t band;
    uint32_t fs_hz;     /* 320 MHz */
    uint32_t nsamples;
} __attribute__((packed));

enum { ST_OK = 0, ST_IOCTL = 1, ST_TIMEOUT = 2, ST_NODATA = 3, ST_ARGS = 4 };

static int g_sock = -1;
static volatile sig_atomic_t g_stop;
static pid_t g_watchdog = -1;

static int testmode(uint32_t oid)
{
    uint32_t buf[7] = { oid, 0, 0, 0, 0, 0, 0 };
    struct iwreq req;
    memset(&req, 0, sizeof(req));
    strncpy(req.ifr_name, IFNAME, IFNAMSIZ - 1);
    req.u.data.pointer = buf;
    req.u.data.length = sizeof(buf);
    req.u.data.flags = 15;
    return ioctl(g_sock, IOCTL_TESTMODE, &req);
}

/* Schickt ein HQA-Kommando; Antwortdaten landen in resp (ab Datenteil). */
static int hqa(uint16_t type, uint16_t id, const uint32_t *words, int nwords,
               uint8_t *resp, int *resp_len)
{
    static uint8_t frame[HQA_BUF];
    struct iwreq req;
    memset(frame, 0, sizeof(frame));
    uint32_t v = htonl(HQA_MAGIC);
    memcpy(frame, &v, 4);
    uint16_t h[4] = { htons(type), htons(id), htons((uint16_t)(nwords * 4)), htons(0xffff) };
    memcpy(frame + 4, h, 8);
    for (int i = 0; i < nwords; ++i) {
        v = htonl(words[i]);
        memcpy(frame + 12 + 4 * i, &v, 4);
    }
    memset(&req, 0, sizeof(req));
    strncpy(req.ifr_name, IFNAME, IFNAMSIZ - 1);
    req.u.data.pointer = frame;
    req.u.data.length = HQA_BUF;
    if (ioctl(g_sock, IOCTL_QA_TOOL, &req) < 0)
        return -1;
    uint16_t len;
    memcpy(&len, frame + 8, 2);
    len = ntohs(len);
    if (len > HQA_BUF - 12)
        len = HQA_BUF - 12;
    if (resp)
        memcpy(resp, frame + 12, len);
    if (resp_len)
        *resp_len = len;
    return 0;
}

static uint32_t be32(const uint8_t *p)
{
    uint32_t v;
    memcpy(&v, p, 4);
    return ntohl(v);
}

static void msleep(int ms)
{
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000000L };
    nanosleep(&ts, NULL);
}

static void icap_stop(void)
{
    const uint32_t w[] = { 1, 0, 0, 0, ICAP_NODE, ICAP_LEN, ICAP_STOP, 0 };
    hqa(8, 0x1580, w, 8, NULL, NULL);
}

/* WLAN in connman einmal aus- und einschalten, damit sich Treiber/Firmware nach
 * dem Testmodus frisch einrichten (Vorsicht nach einem Tx-Hänger mit Kernel-BUG
 * im MTK-Treiber ~30 min nach einer ICAP-Sitzung, Ursache unbelegt). */
static void wifi_reinit(void)
{
    const char *cm = "dbus-send --system --print-reply --dest=net.connman "
                     "/net/connman/technology/wifi net.connman.Technology.SetProperty "
                     "string:Powered variant:boolean:";
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "%sfalse >/dev/null 2>&1", cm);
    if (system(cmd) != 0)
        return;
    sleep(2);
    snprintf(cmd, sizeof(cmd), "%strue >/dev/null 2>&1", cm);
    if (system(cmd) != 0)
        fprintf(stderr, "wfscope-icap: connman-Wiedereinschalten fehlgeschlagen\n");
}

static void restore(void)
{
    /* Reihenfolge wichtig: jedes HQA-Kommando schaltet den Testmodus selbst
     * wieder ein, daher Testmodus-Abbruch zuletzt. */
    icap_stop();
    testmode(OID_ABORT_TEST_MODE);
    sleep(1);
    wifi_reinit();
}

/* Liest einen IQ-Strom (wf, iq) in out; gibt die Anzahl Samples zurück. */
static int fetch(uint32_t wf, uint32_t iq, int16_t *out, int max)
{
    static uint8_t r[HQA_BUF];
    int n = 0, len = 0;
    const uint32_t w[] = { 3, wf, iq };
    for (;;) {
        if (hqa(8, 0x1580, w, 3, r, &len) < 0 || len < 18)
            return n;
        uint32_t cnt = be32(r + 14);
        if (cnt == 0)
            return n;
        if ((int)(18 + 4 * cnt) > len)
            cnt = (len - 18) / 4;
        for (uint32_t i = 0; i < cnt && n < max; ++i) {
            int32_t s = (int32_t)be32(r + 18 + 4 * i);
            out[n++] = (int16_t)(s > 32767 ? 32767 : s < -32768 ? -32768 : s);
        }
    }
}

static int capture(uint32_t ch, uint32_t bw, uint32_t band, int16_t *iq, int *n)
{
    static int16_t si[MAX_SAMPLES], sq[MAX_SAMPLES];
    uint8_t r[64];
    int len;
    const uint32_t bandmode[] = { 1, band };
    const uint32_t rxpath[] = { 1, 0 };
    const uint32_t setch[] = { 1, 8, 0, ch, 0, bw, bw, 0, 0, band, 0 };
    const uint32_t startrx[] = { 4, 7, 0, 0, 0, 0, 0, 0x10000, 0, 0 };
    const uint32_t start[] = { 1, 1, 0, 0, ICAP_NODE, ICAP_LEN, ICAP_STOP, bw };
    const uint32_t status[] = { 2 };

    *n = 0;
    if (hqa(5, 0x152c, bandmode, 2, NULL, NULL) < 0 || hqa(5, 0x100c, rxpath, 2, NULL, NULL) < 0
        || hqa(5, 0x1600, setch, 11, NULL, NULL) < 0 || hqa(5, 0x1600, startrx, 10, NULL, NULL) < 0)
        return ST_IOCTL;
    icap_stop();
    if (hqa(8, 0x1580, start, 8, NULL, NULL) < 0)
        return ST_IOCTL;
    int done = 0;
    for (int i = 0; i < 150 && !done; ++i) { /* bis 3 s */
        msleep(20);
        if (hqa(8, 0x1580, status, 1, r, &len) < 0)
            return ST_IOCTL;
        done = len >= 2 && r[0] == 0 && r[1] == 0;
    }
    if (!done)
        return ST_TIMEOUT;
    int ni = fetch(0, 0, si, MAX_SAMPLES);
    int nq = fetch(0, 1, sq, MAX_SAMPLES);
    int m = ni < nq ? ni : nq;
    if (m == 0)
        return ST_NODATA;
    for (int i = 0; i < m; ++i) {
        iq[2 * i] = si[i];
        iq[2 * i + 1] = sq[i];
    }
    *n = m;
    return ST_OK;
}

static int write_all(const void *p, size_t len)
{
    const uint8_t *b = p;
    while (len) {
        ssize_t w = write(STDOUT_FILENO, b, len);
        if (w < 0) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        b += w;
        len -= (size_t)w;
    }
    return 0;
}

static void on_signal(int sig)
{
    (void)sig;
    g_stop = 1;
}

/* Abgekoppelter Watchdog: räumt auf, sobald der Helfer verschwindet. */
static void spawn_watchdog(void)
{
    pid_t helper = getpid();
    pid_t pid = fork();
    if (pid != 0) {
        g_watchdog = pid;
        return;
    }
    setsid();
    for (int fd = 0; fd < 3; ++fd)
        close(fd);
    while (kill(helper, 0) == 0 || errno == EPERM)
        sleep(1);
    restore();
    _exit(0);
}

int main(void)
{
    static int16_t iq[2 * MAX_SAMPLES];
    char line[128];

    signal(SIGPIPE, SIG_IGN);
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_signal;
    sigaction(SIGTERM, &sa, NULL);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGHUP, &sa, NULL);

    g_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (g_sock < 0) {
        perror("socket");
        return 1;
    }
    if (testmode(OID_TEST_ICAP_MODE) < 0) {
        perror("ICAP-Testmodus");
        return 1;
    }
    spawn_watchdog();
    msleep(500);
    fprintf(stderr, "wfscope-icap: bereit\n");

    while (!g_stop && fgets(line, sizeof(line), stdin)) {
        unsigned ch, bw, band;
        struct frame_hdr h;
        memset(&h, 0, sizeof(h));
        memcpy(h.magic, "WFIQ", 4);
        h.version = 1;
        h.fs_hz = 320000000u;
        int n = 0;
        if (strncmp(line, "quit", 4) == 0)
            break;
        if (sscanf(line, "cap %u %u %u", &ch, &bw, &band) != 3) {
            h.status = ST_ARGS;
        } else {
            h.channel = ch;
            h.bw = bw;
            h.band = band;
            h.status = (uint16_t)capture(ch, bw, band, iq, &n);
        }
        h.nsamples = (uint32_t)n;
        if (write_all(&h, sizeof(h)) < 0 || (n && write_all(iq, (size_t)n * 4) < 0))
            break;
    }
    if (g_watchdog > 0)
        kill(g_watchdog, SIGKILL); /* sonst räumt er ein zweites Mal auf */
    restore();
    fprintf(stderr, "wfscope-icap: WLAN wiederhergestellt\n");
    return 0;
}
