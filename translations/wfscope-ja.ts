<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1">
<context>
    <name>AboutPage</name>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="18"/>
        <source>About wfscope</source>
        <translation>wfscope について</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="34"/>
        <source>Version %1</source>
        <translation>バージョン %1</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="44"/>
        <source>Spectrum analyser and waterfall for Sailfish OS: internal FM radio with RDS, HackRF and SDRplay RSPdx (also as panadapter), Wi-Fi spectrum via ICAP.</source>
        <translation>Sailfish OS 用のスペクトラムアナライザーとウォーターフォール：RDS 対応の内蔵 FM ラジオ、HackRF と SDRplay RSPdx（パンアダプターとしても）、ICAP による Wi-Fi スペクトラム。</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="53"/>
        <source>Early stage and very experimental.</source>
        <translation>開発初期の段階で、非常に実験的です。</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="56"/>
        <source>Reception tips (internal FM)</source>
        <translation>受信のヒント（内蔵 FM）</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="63"/>
        <source>• A USB-C cable plugged into the phone acts as the antenna. The antenna signal runs on the SBU1/SBU2 pins (A8/B8), so the cable must carry the SBU wires: fully wired USB 3.x/USB4 cables do (e.g. 40 Gbps), simple charging and USB 2.0 cables do not. Tested: 1.3 m USB4 cable with e-marker.
• Let the cable hang straight; coiled up it receives much worse.
• Leave the other end free: a connected laptop adds a lot of noise.
• Without such a cable reception is very weak; station names (RDS) need a good signal.</source>
        <translation>• 電話に挿した USB-C ケーブルがアンテナとして働きます。アンテナ信号は SBU1/SBU2 ピン（A8/B8） を通るため、ケーブルには SBU 線が必要です。全結線の USB 3.x/USB4 ケーブル（例：40 Gbps）には含まれますが、簡易な充電用ケーブルや USB 2.0 ケーブルには含まれません。テスト済み：e-marker 付き 1.3 m の USB4 ケーブル。
• ケーブルはまっすぐ垂らしてください。巻いたままだと受信がかなり悪くなります。
• もう一方の端は何もつながないでください。ノートパソコンを接続するとノイズが大きく増えます。
• このようなケーブルがないと受信は非常に弱く、局名（RDS）の表示には良好な信号が必要です。</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="66"/>
        <source>Warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="73"/>
        <source>The Wi-Fi spectrum (ICAP) puts the Wi-Fi chip into test mode and switches Wi-Fi off. On the Jolla J2 it has crashed the device; use it only if you can live with a reboot.</source>
        <translation>Wi-Fi スペクトラム（ICAP）は Wi-Fi チップをテストモードにし、Wi-Fi をオフにします。Jolla J2 では端末がクラッシュしたことがあります。再起動しても問題ない場合のみ使用してください。</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="76"/>
        <location filename="../qml/pages/AboutPage.qml" line="80"/>
        <source>Language</source>
        <translation>言語</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="81"/>
        <source>Takes effect at the next start</source>
        <translation>次回起動時に反映されます</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="97"/>
        <source>License</source>
        <translation>ライセンス</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="104"/>
        <source>GNU General Public License v3 or later (GPLv3+), with an exception that allows using the proprietary SDRplay API at runtime.
Contains tinyalsa and libhackrf (BSD licences). The SDRplay API is not part of the package.</source>
        <translation>GNU General Public License v3 以降（GPLv3+）。実行時にプロプライエタリな SDRplay API を使用することを認める例外付き。
tinyalsa と libhackrf（BSD ライセンス）を含みます。SDRplay API はパッケージに含まれていません。</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="108"/>
        <source>Source code on GitHub</source>
        <translation>GitHub のソースコード</translation>
    </message>
</context>
<context>
    <name>CoverPage</name>
    <message>
        <location filename="../qml/cover/CoverPage.qml" line="21"/>
        <source>running</source>
        <translation>実行中</translation>
    </message>
    <message>
        <location filename="../qml/cover/CoverPage.qml" line="21"/>
        <source>ready</source>
        <translation>準備完了</translation>
    </message>
</context>
<context>
    <name>FmAudio</name>
    <message>
        <location filename="../src/fmaudio.cpp" line="110"/>
        <source>ALSA mixer not accessible</source>
        <translation>ALSA ミキサーにアクセスできません</translation>
    </message>
    <message>
        <location filename="../src/fmaudio.cpp" line="160"/>
        <source>FM capture: %1</source>
        <translation>FM キャプチャ：%1</translation>
    </message>
</context>
<context>
    <name>FmSource</name>
    <message>
        <location filename="../src/fmsource.cpp" line="126"/>
        <source>Listening %1 MHz%3 · %2 dBm</source>
        <translation>受信中 %1 MHz%3 · %2 dBm</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="140"/>
        <source>Internal FM ready — built-in short antenna</source>
        <translation>内蔵 FM 準備完了 — 内蔵ショートアンテナ</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="342"/>
        <source>The FM chip cannot be powered on</source>
        <translation>FM チップの電源を入れられません</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="327"/>
        <location filename="../src/fmsource.cpp" line="353"/>
        <source>Internal FM scanning …</source>
        <translation>内蔵 FM スキャン中 …</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="381"/>
        <source>Internal FM stopped</source>
        <translation>内蔵 FM 停止</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="621"/>
        <source>FM measurement failed (%1)</source>
        <translation>FM 測定に失敗しました（%1）</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="637"/>
        <source>Internal FM · %1 channels · %2 s/sweep · %3</source>
        <translation>内蔵 FM · %1 チャンネル · %2 秒/スイープ · %3</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="640"/>
        <source>short antenna</source>
        <translation>ショートアンテナ</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="640"/>
        <source>headset antenna</source>
        <translation>ヘッドセットアンテナ</translation>
    </message>
    <message>
        <location filename="../src/fmsource.h" line="53"/>
        <source>FM radio (internal)</source>
        <translation>FM ラジオ（内蔵）</translation>
    </message>
    <message>
        <location filename="../src/fmsource.h" line="60"/>
        <source>FM 87.5–108</source>
        <translation>FM 87.5–108</translation>
    </message>
</context>
<context>
    <name>HackRfSource</name>
    <message>
        <location filename="../src/hackrfsource.cpp" line="25"/>
        <source>HackRF ready — connect the device via USB OTG</source>
        <translation>HackRF 準備完了 — USB OTG でデバイスを接続してください</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="90"/>
        <source>No HackRF found (OTG cable, power?)</source>
        <translation>HackRF が見つかりません（OTG ケーブル、電源は？）</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="92"/>
        <source>HackRF: no USB access (udev rule missing? try as root)</source>
        <translation>HackRF：USB アクセスなし（udev ルールがない？root で試してください）</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="94"/>
        <source>Opening HackRF: %1</source>
        <translation>HackRF を開いています：%1</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="114"/>
        <source>HackRF stopped</source>
        <translation>HackRF 停止</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="148"/>
        <source>HackRF sweep: %1</source>
        <translation>HackRF スイープ：%1</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="151"/>
        <source>HackRF sweeping %1–%2 MHz · LNA %3 · VGA %4%5</source>
        <translation>HackRF スイープ中 %1–%2 MHz · LNA %3 · VGA %4%5</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.h" line="31"/>
        <source>2.4 GHz Wi-Fi</source>
        <translation>2.4 GHz Wi-Fi</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.h" line="32"/>
        <source>5 GHz Wi-Fi</source>
        <translation>5 GHz Wi-Fi</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.h" line="33"/>
        <source>2.4 + 5 GHz (2.4–5.9)</source>
        <translation>2.4 + 5 GHz（2.4–5.9）</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.h" line="34"/>
        <source>FM 87.5–108</source>
        <translation>FM 87.5–108</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.h" line="36"/>
        <source>Overview 1–6000</source>
        <translation>概要 1–6000</translation>
    </message>
</context>
<context>
    <name>IcapSource</name>
    <message>
        <location filename="../src/icapsource.cpp" line="78"/>
        <source>ICAP ready — starting switches Wi-Fi off</source>
        <translation>ICAP 準備完了 — 開始すると Wi-Fi がオフになります</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="101"/>
        <source>Helper missing: %1</source>
        <translation>ヘルパーがありません：%1</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="108"/>
        <source>ICAP starting (Wi-Fi off) …</source>
        <translation>ICAP 開始中（Wi-Fi オフ）…</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="123"/>
        <source>Stopping, restoring Wi-Fi …</source>
        <translation>停止中、Wi-Fi を復元しています …</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="159"/>
        <source>The range contains no Wi-Fi band (2.4 / 5 / 6 GHz)</source>
        <translation>範囲に Wi-Fi バンド（2.4 / 5 / 6 GHz）が含まれていません</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="176"/>
        <source>ICAP active (Wi-Fi off)</source>
        <translation>ICAP 動作中（Wi-Fi オフ）</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="190"/>
        <source>Protocol error from the helper</source>
        <translation>ヘルパーからのプロトコルエラー</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="216"/>
        <source>Capture channel %1: %2</source>
        <translation>チャンネル %1 のキャプチャ：%2</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="286"/>
        <source>ICAP active (Wi-Fi off) · %1 chunks · %2 s/sweep%3</source>
        <translation>ICAP 動作中（Wi-Fi オフ） · %1 チャンク · %2 秒/スイープ%3</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="288"/>
        <source> · %1 errors</source>
        <translation> · エラー %1 件</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="296"/>
        <source>Stopped, Wi-Fi restored</source>
        <translation>停止しました。Wi-Fi は復元されました</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="298"/>
        <source>pkexec: not authorized (polkit rule installed?)</source>
        <translation>pkexec：権限がありません（polkit ルールはインストール済み？）</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="300"/>
        <source>Helper crashed — the watchdog restores Wi-Fi</source>
        <translation>ヘルパーがクラッシュしました — ウォッチドッグが Wi-Fi を復元します</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="302"/>
        <source>Helper exited (code %1)</source>
        <translation>ヘルパーが終了しました（コード %1）</translation>
    </message>
    <message>
        <location filename="../src/icapsource.h" line="20"/>
        <source>Wi-Fi chip (ICAP)</source>
        <translation>Wi-Fi チップ（ICAP）</translation>
    </message>
    <message>
        <location filename="../src/icapsource.h" line="27"/>
        <source>2.4 GHz</source>
        <translation>2.4 GHz</translation>
    </message>
    <message>
        <location filename="../src/icapsource.h" line="30"/>
        <source>6 GHz low (untested)</source>
        <translation>6 GHz 下側（未検証）</translation>
    </message>
</context>
<context>
    <name>LanguageSettings</name>
    <message>
        <location filename="../src/languagesettings.cpp" line="57"/>
        <source>Follow the device</source>
        <translation>デバイスに従う</translation>
    </message>
</context>
<context>
    <name>ScopePage</name>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="135"/>
        <source>Not listened to long enough yet. Listen to the station for about 20 seconds so that its name can be read via RDS.</source>
        <translation>まだ受信時間が足りません。RDS で局名を読み取れるよう、約 20 秒間この局を受信してください。</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="137"/>
        <source>No RDS data received in %1 s (level %2 dBm). The signal is too weak for RDS, or the station does not send RDS.</source>
        <translation>%1 秒間 RDS データを受信できませんでした（レベル %2 dBm）。信号が RDS には弱すぎるか、局が RDS を送信していません。</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="140"/>
        <source>RDS data received (%1 times), but too disturbed to identify the station. The signal is too weak for RDS.</source>
        <translation>RDS データを受信しました（%1 回）が、乱れが大きく局を識別できません。信号が RDS には弱すぎます。</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="142"/>
        <source>Station identified (PI %1), but its name has not been received yet. Keep listening a little longer.</source>
        <translation>局を識別しました（PI %1）が、局名はまだ受信していません。もう少し受信を続けてください。</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="149"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="153"/>
        <source>Name for %1 MHz</source>
        <translation>%1 MHz の局名</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="154"/>
        <source>No name for %1 MHz</source>
        <translation>%1 MHz の局名なし</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="172"/>
        <source>Station name</source>
        <translation>局名</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="173"/>
        <source>Enter the station name</source>
        <translation>局名を入力</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="184"/>
        <source>Names received via RDS are learned again when you listen to the station.</source>
        <translation>RDS で受信した局名は、その局を聴くと再び取得されます。</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="186"/>
        <source>The name is stored together with the station code (PI %1). Where another station uses this frequency, it is dropped again.</source>
        <translation>局名は局コード（PI %1）と一緒に保存されます。別の局がこの周波数を使う地域では、再び破棄されます。</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="187"/>
        <source>Without a station code (PI) the name is stored for this frequency only. If RDS later identifies a station here, its name takes precedence.</source>
        <translation>局コード（PI）がない場合、局名はこの周波数にのみ保存されます。後で RDS がここで局を識別すると、その局名が優先されます。</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="192"/>
        <source>Delete name</source>
        <translation>局名を削除</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="215"/>
        <source>Tune</source>
        <translation>チューニング</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="220"/>
        <source>Frequency in MHz</source>
        <translation>周波数（MHz）</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="233"/>
        <source>Range of this source: %1 – %2 MHz</source>
        <translation>このソースの範囲：%1 – %2 MHz</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="252"/>
        <source>About wfscope</source>
        <translation>wfscope について</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="271"/>
        <source>Stop</source>
        <translation>停止</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="271"/>
        <source>Start</source>
        <translation>開始</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="342"/>
        <source>Step</source>
        <translation>ステップ</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="351"/>
        <source>Span %1 · pinch the spectrum to zoom</source>
        <translation>スパン %1 · スペクトラムをピンチしてズーム</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="538"/>
        <source>Frequencies</source>
        <translation>周波数</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="544"/>
        <source>Names</source>
        <translation>局名</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="551"/>
        <source>Scan</source>
        <translation>スキャン</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="557"/>
        <source>Snap</source>
        <translation>吸着</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="569"/>
        <source>Intensity</source>
        <translation>強度</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="580"/>
        <source>Peak
%1 MHz</source>
        <translation>ピーク
%1 MHz</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="596"/>
        <source>Source</source>
        <translation>ソース</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="610"/>
        <source>Band</source>
        <translation>バンド</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="614"/>
        <source>custom: %1 – %2 MHz · choosing a band jumps back</source>
        <translation>カスタム：%1 – %2 MHz · バンドを選ぶと戻ります</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="630"/>
        <source>FM radio (internal)</source>
        <translation>FM ラジオ（内蔵）</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="632"/>
        <source>Built-in short antenna</source>
        <translation>内蔵ショートアンテナ</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="633"/>
        <source>Off: antenna via a headset (if the device supports one)</source>
        <translation>オフ：ヘッドセット経由のアンテナ（デバイスが対応している場合）</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="638"/>
        <source>Listen</source>
        <translation>受信</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="644"/>
        <source>Slide a station under the red line or tap it; scanning pauses</source>
        <translation>局を赤い線の下へスライドするかタップしてください。スキャンは一時停止します</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="671"/>
        <source>Volume</source>
        <translation>音量</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="677"/>
        <source>Stations</source>
        <translation>局</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="708"/>
        <source>Level per 100 kHz channel (no IQ), about 50 ms per channel; ● = recognised as a station by the chip. Runs without root, Wi-Fi stays on.</source>
        <translation>100 kHz チャンネルごとのレベル（IQ なし）、1 チャンネルあたり約 50 ms。● = チップが局として認識。root 不要で、Wi-Fi はオンのままです。</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="721"/>
        <source>LNA (IF gain)</source>
        <translation>LNA（IF ゲイン）</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="729"/>
        <source>VGA (baseband gain)</source>
        <translation>VGA（ベースバンドゲイン）</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="734"/>
        <source>RF preamp (+14 dB)</source>
        <translation>RF プリアンプ（+14 dB）</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="735"/>
        <source>Only for weak signals; overloads easily</source>
        <translation>弱い信号専用。過大入力になりやすい</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="747"/>
        <source>Panadapter FT-991A</source>
        <translation>パンアダプター FT-991A</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="748"/>
        <source>Tuner fixed on the IF (TSP IFace 2), axis in VFO frequency</source>
        <translation>チューナーを IF に固定（TSP IFace 2）、軸は VFO 周波数</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="755"/>
        <source>VFO frequency (MHz)</source>
        <translation>VFO 周波数（MHz）</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="764"/>
        <source>First IF of the radio (MHz)</source>
        <translation>無線機の第 1 IF（MHz）</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="775"/>
        <source>IF fine correction</source>
        <translation>IF 微調整</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="781"/>
        <source>Mirror spectrum</source>
        <translation>スペクトラムを反転</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="782"/>
        <source>If signals move the wrong way when turning the VFO</source>
        <translation>VFO を回したときに信号が逆方向に動く場合</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="788"/>
        <source>Antenna input</source>
        <translation>アンテナ入力</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="801"/>
        <source>LNA state (0 = max. gain)</source>
        <translation>LNA ステート（0 = 最大ゲイン）</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="806"/>
        <source>AGC</source>
        <translation>AGC</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="812"/>
        <source>Display</source>
        <translation>表示</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="816"/>
        <source>Auto contrast</source>
        <translation>自動コントラスト</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="817"/>
        <source>Colour scale follows the noise floor</source>
        <translation>カラースケールがノイズフロアに追従します</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="832"/>
        <source>Noise floor</source>
        <translation>ノイズフロア</translation>
    </message>
</context>
<context>
    <name>SdrplaySource</name>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="45"/>
        <source>RSPdx ready — connect the device via USB OTG</source>
        <translation>RSPdx 準備完了 — USB OTG でデバイスを接続してください</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="65"/>
        <source>SDRplay API not installed (libsdrplay_api.so.3)</source>
        <translation>SDRplay API がインストールされていません（libsdrplay_api.so.3）</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="90"/>
        <source>SDRplay API incomplete (wrong version?)</source>
        <translation>SDRplay API が不完全です（バージョン違い？）</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="103"/>
        <source>SDRplay service unreachable (is sdrplay_apiService running?)</source>
        <translation>SDRplay サービスに接続できません（sdrplay_apiService は動作中？）</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="121"/>
        <source>No SDRplay device found (OTG cable?)</source>
        <translation>SDRplay デバイスが見つかりません（OTG ケーブルは？）</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="175"/>
        <source>RSPdx panadapter · IF %1 MHz · VFO %2 MHz</source>
        <translation>RSPdx パンアダプター · IF %1 MHz · VFO %2 MHz</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="177"/>
        <source>RSPdx receiving · %1 step(s)</source>
        <translation>RSPdx 受信中 · %1 ステップ</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="188"/>
        <source>RSPdx stopped</source>
        <translation>RSPdx 停止</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="337"/>
        <source>RSPdx was unplugged</source>
        <translation>RSPdx が取り外されました</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.h" line="39"/>
        <source>FM 87.5–108</source>
        <translation>FM 87.5–108</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.h" line="40"/>
        <source>Airband 118–137</source>
        <translation>航空無線 118–137</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.h" line="43"/>
        <source>40 m (7.0–7.2)</source>
        <translation>40 m（7.0–7.2）</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.h" line="44"/>
        <source>20 m (14.0–14.35)</source>
        <translation>20 m（14.0–14.35）</translation>
    </message>
</context>
<context>
    <name>SyntheticSource</name>
    <message>
        <location filename="../src/syntheticsource.cpp" line="8"/>
        <source>synthetic</source>
        <translation>合成</translation>
    </message>
    <message>
        <location filename="../src/syntheticsource.h" line="13"/>
        <source>Synthetic (demo)</source>
        <translation>合成（デモ）</translation>
    </message>
    <message>
        <location filename="../src/syntheticsource.h" line="18"/>
        <source>2.4 GHz</source>
        <translation>2.4 GHz</translation>
    </message>
</context>
</TS>
