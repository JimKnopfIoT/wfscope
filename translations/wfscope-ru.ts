<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1">
<context>
    <name>AboutPage</name>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="18"/>
        <source>About wfscope</source>
        <translation>О wfscope</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="34"/>
        <source>Version %1</source>
        <translation>Версия %1</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="44"/>
        <source>Spectrum analyser and waterfall for Sailfish OS: internal FM radio with RDS, HackRF and SDRplay RSPdx (also as panadapter), Wi-Fi spectrum via ICAP.</source>
        <translation>Анализатор спектра и водопад для Sailfish OS: встроенное FM-радио с RDS, HackRF и SDRplay RSPdx (в том числе как панорамная приставка), спектр Wi-Fi через ICAP.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="53"/>
        <source>Early stage and very experimental.</source>
        <translation>Ранняя стадия, очень экспериментально.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="56"/>
        <source>Reception tips (internal FM)</source>
        <translation>Советы по приёму (встроенное FM)</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="63"/>
        <source>• A USB-C cable plugged into the phone acts as the antenna. The antenna signal runs on the SBU1/SBU2 pins (A8/B8), so the cable must carry the SBU wires: fully wired USB 3.x/USB4 cables do (e.g. 40 Gbps), simple charging and USB 2.0 cables do not. Tested: 1.3 m USB4 cable with e-marker.
• Let the cable hang straight; coiled up it receives much worse.
• Leave the other end free: a connected laptop adds a lot of noise.
• Without such a cable reception is very weak; station names (RDS) need a good signal.</source>
        <translation>• Подключённый к телефону кабель USB-C служит антенной. Сигнал антенны идёт по контактам SBU1/SBU2 (A8/B8), поэтому в кабеле должны быть линии SBU: в полнофункциональных кабелях USB 3.x/USB4 они есть (например 40 Гбит/с), в простых зарядных кабелях и кабелях USB 2.0 — нет. Проверено: кабель USB4 длиной 1,3 м с e-marker.
• Кабель должен свисать прямо; свёрнутый, он принимает гораздо хуже.
• Оставьте другой конец свободным: подключённый ноутбук добавляет много шума.
• Без такого кабеля приём очень слабый; для названий станций (RDS) нужен хороший сигнал.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="66"/>
        <source>Warning</source>
        <translation>Предупреждение</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="73"/>
        <source>The Wi-Fi spectrum (ICAP) puts the Wi-Fi chip into test mode and switches Wi-Fi off. On the Jolla J2 it has crashed the device; use it only if you can live with a reboot.</source>
        <translation>Спектр Wi-Fi (ICAP) переводит чип Wi-Fi в тестовый режим и отключает Wi-Fi. На Jolla J2 это уже приводило к сбою устройства; используйте, только если перезагрузка вас не смущает.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="76"/>
        <location filename="../qml/pages/AboutPage.qml" line="80"/>
        <source>Language</source>
        <translation>Язык</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="81"/>
        <source>Takes effect at the next start</source>
        <translation>Вступит в силу при следующем запуске</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="97"/>
        <source>License</source>
        <translation>Лицензия</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="104"/>
        <source>GNU General Public License v3 or later (GPLv3+), with an exception that allows using the proprietary SDRplay API at runtime.
Contains tinyalsa and libhackrf (BSD licences). The SDRplay API is not part of the package.</source>
        <translation>GNU General Public License v3 или новее (GPLv3+), с исключением, разрешающим использование проприетарного API SDRplay во время выполнения.
Содержит tinyalsa и libhackrf (лицензии BSD). API SDRplay не входит в пакет.</translation>
    </message>
    <message>
        <location filename="../qml/pages/AboutPage.qml" line="108"/>
        <source>Source code on GitHub</source>
        <translation>Исходный код на GitHub</translation>
    </message>
</context>
<context>
    <name>CoverPage</name>
    <message>
        <location filename="../qml/cover/CoverPage.qml" line="21"/>
        <source>running</source>
        <translation>работает</translation>
    </message>
    <message>
        <location filename="../qml/cover/CoverPage.qml" line="21"/>
        <source>ready</source>
        <translation>готово</translation>
    </message>
</context>
<context>
    <name>FmAudio</name>
    <message>
        <location filename="../src/fmaudio.cpp" line="102"/>
        <source>ALSA mixer not accessible</source>
        <translation>Микшер ALSA недоступен</translation>
    </message>
    <message>
        <location filename="../src/fmaudio.cpp" line="152"/>
        <source>FM capture: %1</source>
        <translation>Захват FM: %1</translation>
    </message>
</context>
<context>
    <name>FmSource</name>
    <message>
        <location filename="../src/fmsource.cpp" line="126"/>
        <source>Listening %1 MHz%3 · %2 dBm</source>
        <translation>Прослушивание %1 MHz%3 · %2 dBm</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="140"/>
        <source>Internal FM ready — built-in short antenna</source>
        <translation>Встроенное FM готово — встроенная короткая антенна</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="342"/>
        <source>The FM chip cannot be powered on</source>
        <translation>Не удаётся включить FM-чип</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="327"/>
        <location filename="../src/fmsource.cpp" line="353"/>
        <source>Internal FM scanning …</source>
        <translation>Встроенное FM сканирует …</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="381"/>
        <source>Internal FM stopped</source>
        <translation>Встроенное FM остановлено</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="621"/>
        <source>FM measurement failed (%1)</source>
        <translation>Ошибка измерения FM (%1)</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="637"/>
        <source>Internal FM · %1 channels · %2 s/sweep · %3</source>
        <translation>Встроенное FM · %1 каналов · %2 с/проход · %3</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="640"/>
        <source>short antenna</source>
        <translation>короткая антенна</translation>
    </message>
    <message>
        <location filename="../src/fmsource.cpp" line="640"/>
        <source>headset antenna</source>
        <translation>антенна гарнитуры</translation>
    </message>
    <message>
        <location filename="../src/fmsource.h" line="53"/>
        <source>FM radio (internal)</source>
        <translation>FM-радио (встроенное)</translation>
    </message>
    <message>
        <location filename="../src/fmsource.h" line="60"/>
        <source>FM 87.5–108</source>
        <translation>FM 87,5–108</translation>
    </message>
</context>
<context>
    <name>HackRfSource</name>
    <message>
        <location filename="../src/hackrfsource.cpp" line="25"/>
        <source>HackRF ready — connect the device via USB OTG</source>
        <translation>HackRF готов — подключите устройство через USB OTG</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="90"/>
        <source>No HackRF found (OTG cable, power?)</source>
        <translation>HackRF не найден (кабель OTG, питание?)</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="92"/>
        <source>HackRF: no USB access (udev rule missing? try as root)</source>
        <translation>HackRF: нет доступа к USB (нет правила udev? попробуйте от root)</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="94"/>
        <source>Opening HackRF: %1</source>
        <translation>Открытие HackRF: %1</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="114"/>
        <source>HackRF stopped</source>
        <translation>HackRF остановлен</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="148"/>
        <source>HackRF sweep: %1</source>
        <translation>Проход HackRF: %1</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.cpp" line="151"/>
        <source>HackRF sweeping %1–%2 MHz · LNA %3 · VGA %4%5</source>
        <translation>HackRF сканирует %1–%2 MHz · LNA %3 · VGA %4%5</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.h" line="31"/>
        <source>2.4 GHz Wi-Fi</source>
        <translation>Wi-Fi 2,4 GHz</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.h" line="32"/>
        <source>5 GHz Wi-Fi</source>
        <translation>Wi-Fi 5 GHz</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.h" line="33"/>
        <source>2.4 + 5 GHz (2.4–5.9)</source>
        <translation>2,4 + 5 GHz (2,4–5,9)</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.h" line="34"/>
        <source>FM 87.5–108</source>
        <translation>FM 87,5–108</translation>
    </message>
    <message>
        <location filename="../src/hackrfsource.h" line="36"/>
        <source>Overview 1–6000</source>
        <translation>Обзор 1–6000</translation>
    </message>
</context>
<context>
    <name>IcapSource</name>
    <message>
        <location filename="../src/icapsource.cpp" line="78"/>
        <source>ICAP ready — starting switches Wi-Fi off</source>
        <translation>ICAP готов — запуск отключит Wi-Fi</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="101"/>
        <source>Helper missing: %1</source>
        <translation>Отсутствует помощник: %1</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="108"/>
        <source>ICAP starting (Wi-Fi off) …</source>
        <translation>ICAP запускается (Wi-Fi выкл.) …</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="123"/>
        <source>Stopping, restoring Wi-Fi …</source>
        <translation>Остановка, восстановление Wi-Fi …</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="159"/>
        <source>The range contains no Wi-Fi band (2.4 / 5 / 6 GHz)</source>
        <translation>Диапазон не содержит полос Wi-Fi (2,4 / 5 / 6 GHz)</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="176"/>
        <source>ICAP active (Wi-Fi off)</source>
        <translation>ICAP активен (Wi-Fi выкл.)</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="190"/>
        <source>Protocol error from the helper</source>
        <translation>Ошибка протокола помощника</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="216"/>
        <source>Capture channel %1: %2</source>
        <translation>Захват канала %1: %2</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="286"/>
        <source>ICAP active (Wi-Fi off) · %1 chunks · %2 s/sweep%3</source>
        <translation>ICAP активен (Wi-Fi выкл.) · %1 фрагм. · %2 с/проход%3</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="288"/>
        <source> · %1 errors</source>
        <translation> · ошибок: %1</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="296"/>
        <source>Stopped, Wi-Fi restored</source>
        <translation>Остановлено, Wi-Fi восстановлен</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="298"/>
        <source>pkexec: not authorized (polkit rule installed?)</source>
        <translation>pkexec: нет авторизации (правило polkit установлено?)</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="300"/>
        <source>Helper crashed — the watchdog restores Wi-Fi</source>
        <translation>Сбой помощника — сторожевой таймер восстанавливает Wi-Fi</translation>
    </message>
    <message>
        <location filename="../src/icapsource.cpp" line="302"/>
        <source>Helper exited (code %1)</source>
        <translation>Помощник завершился (код %1)</translation>
    </message>
    <message>
        <location filename="../src/icapsource.h" line="20"/>
        <source>Wi-Fi chip (ICAP)</source>
        <translation>Чип Wi-Fi (ICAP)</translation>
    </message>
    <message>
        <location filename="../src/icapsource.h" line="27"/>
        <source>2.4 GHz</source>
        <translation>2,4 GHz</translation>
    </message>
    <message>
        <location filename="../src/icapsource.h" line="30"/>
        <source>6 GHz low (untested)</source>
        <translation>6 GHz нижний (не проверено)</translation>
    </message>
</context>
<context>
    <name>LanguageSettings</name>
    <message>
        <location filename="../src/languagesettings.cpp" line="57"/>
        <source>Follow the device</source>
        <translation>Как в устройстве</translation>
    </message>
</context>
<context>
    <name>ScopePage</name>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="135"/>
        <source>Not listened to long enough yet. Listen to the station for about 20 seconds so that its name can be read via RDS.</source>
        <translation>Прослушано ещё недостаточно долго. Слушайте станцию около 20 секунд, чтобы её название можно было прочитать через RDS.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="137"/>
        <source>No RDS data received in %1 s (level %2 dBm). The signal is too weak for RDS, or the station does not send RDS.</source>
        <translation>За %1 с не получено данных RDS (уровень %2 dBm). Сигнал слишком слаб для RDS, или станция не передаёт RDS.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="140"/>
        <source>RDS data received (%1 times), but too disturbed to identify the station. The signal is too weak for RDS.</source>
        <translation>Данные RDS получены (раз: %1), но слишком искажены, чтобы определить станцию. Сигнал слишком слаб для RDS.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="142"/>
        <source>Station identified (PI %1), but its name has not been received yet. Keep listening a little longer.</source>
        <translation>Станция определена (PI %1), но её название ещё не получено. Послушайте ещё немного.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="149"/>
        <source>Save</source>
        <translation>Сохранить</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="153"/>
        <source>Name for %1 MHz</source>
        <translation>Название для %1 MHz</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="154"/>
        <source>No name for %1 MHz</source>
        <translation>Нет названия для %1 MHz</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="172"/>
        <source>Station name</source>
        <translation>Название станции</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="173"/>
        <source>Enter the station name</source>
        <translation>Введите название станции</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="184"/>
        <source>Names received via RDS are learned again when you listen to the station.</source>
        <translation>Названия, полученные через RDS, будут снова получены, когда вы слушаете станцию.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="186"/>
        <source>The name is stored together with the station code (PI %1). Where another station uses this frequency, it is dropped again.</source>
        <translation>Название сохраняется вместе с кодом станции (PI %1). Там, где эту частоту использует другая станция, оно снова отбрасывается.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="187"/>
        <source>Without a station code (PI) the name is stored for this frequency only. If RDS later identifies a station here, its name takes precedence.</source>
        <translation>Без кода станции (PI) название сохраняется только для этой частоты. Если позже RDS определит здесь станцию, приоритет будет у её названия.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="192"/>
        <source>Delete name</source>
        <translation>Удалить название</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="215"/>
        <source>Tune</source>
        <translation>Настройка</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="220"/>
        <source>Frequency in MHz</source>
        <translation>Частота в MHz</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="233"/>
        <source>Range of this source: %1 – %2 MHz</source>
        <translation>Диапазон источника: %1 – %2 MHz</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="252"/>
        <source>About wfscope</source>
        <translation>О wfscope</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="271"/>
        <source>Stop</source>
        <translation>Стоп</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="271"/>
        <source>Start</source>
        <translation>Старт</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="342"/>
        <source>Step</source>
        <translation>Шаг</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="351"/>
        <source>Span %1 · pinch the spectrum to zoom</source>
        <translation>Обзор %1 · масштаб щипком по спектру</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="538"/>
        <source>Frequencies</source>
        <translation>Частоты</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="544"/>
        <source>Names</source>
        <translation>Названия</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="551"/>
        <source>Scan</source>
        <translation>Скан</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="557"/>
        <source>Snap</source>
        <translation>Привязка</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="569"/>
        <source>Intensity</source>
        <translation>Яркость</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="580"/>
        <source>Peak
%1 MHz</source>
        <translation>Пик
%1 MHz</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="596"/>
        <source>Source</source>
        <translation>Источник</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="610"/>
        <source>Band</source>
        <translation>Диапазон</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="614"/>
        <source>custom: %1 – %2 MHz · choosing a band jumps back</source>
        <translation>свой: %1 – %2 MHz · выбор диапазона вернёт назад</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="630"/>
        <source>FM radio (internal)</source>
        <translation>FM-радио (встроенное)</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="632"/>
        <source>Built-in short antenna</source>
        <translation>Встроенная короткая антенна</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="633"/>
        <source>Off: antenna via a headset (if the device supports one)</source>
        <translation>Выкл.: антенна через гарнитуру (если устройство поддерживает)</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="638"/>
        <source>Listen</source>
        <translation>Слушать</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="644"/>
        <source>Slide a station under the red line or tap it; scanning pauses</source>
        <translation>Сдвиньте станцию под красную линию или коснитесь её; сканирование приостановится</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="671"/>
        <source>Volume</source>
        <translation>Громкость</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="677"/>
        <source>Stations</source>
        <translation>Станции</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="708"/>
        <source>Level per 100 kHz channel (no IQ), about 50 ms per channel; ● = recognised as a station by the chip. Runs without root, Wi-Fi stays on.</source>
        <translation>Уровень на канал 100 kHz (без IQ), около 50 мс на канал; ● = чип распознал станцию. Работает без root, Wi-Fi остаётся включённым.</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="721"/>
        <source>LNA (IF gain)</source>
        <translation>LNA (усиление ПЧ)</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="729"/>
        <source>VGA (baseband gain)</source>
        <translation>VGA (усиление основной полосы)</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="734"/>
        <source>RF preamp (+14 dB)</source>
        <translation>Предусилитель ВЧ (+14 dB)</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="735"/>
        <source>Only for weak signals; overloads easily</source>
        <translation>Только для слабых сигналов; легко перегружается</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="747"/>
        <source>Panadapter FT-991A</source>
        <translation>Панорамная приставка FT-991A</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="748"/>
        <source>Tuner fixed on the IF (TSP IFace 2), axis in VFO frequency</source>
        <translation>Тюнер зафиксирован на ПЧ (TSP IFace 2), шкала в частоте VFO</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="755"/>
        <source>VFO frequency (MHz)</source>
        <translation>Частота VFO (MHz)</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="764"/>
        <source>First IF of the radio (MHz)</source>
        <translation>Первая ПЧ трансивера (MHz)</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="775"/>
        <source>IF fine correction</source>
        <translation>Точная коррекция ПЧ</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="781"/>
        <source>Mirror spectrum</source>
        <translation>Зеркальный спектр</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="782"/>
        <source>If signals move the wrong way when turning the VFO</source>
        <translation>Если сигналы смещаются не в ту сторону при вращении VFO</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="788"/>
        <source>Antenna input</source>
        <translation>Антенный вход</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="801"/>
        <source>LNA state (0 = max. gain)</source>
        <translation>Ступень LNA (0 = макс. усиление)</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="806"/>
        <source>AGC</source>
        <translation>AGC</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="812"/>
        <source>Display</source>
        <translation>Отображение</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="816"/>
        <source>Auto contrast</source>
        <translation>Автоконтраст</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="817"/>
        <source>Colour scale follows the noise floor</source>
        <translation>Цветовая шкала следует за уровнем шума</translation>
    </message>
    <message>
        <location filename="../qml/pages/ScopePage.qml" line="832"/>
        <source>Noise floor</source>
        <translation>Уровень шума</translation>
    </message>
</context>
<context>
    <name>SdrplaySource</name>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="45"/>
        <source>RSPdx ready — connect the device via USB OTG</source>
        <translation>RSPdx готов — подключите устройство через USB OTG</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="65"/>
        <source>SDRplay API not installed (libsdrplay_api.so.3)</source>
        <translation>API SDRplay не установлен (libsdrplay_api.so.3)</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="90"/>
        <source>SDRplay API incomplete (wrong version?)</source>
        <translation>API SDRplay неполный (неверная версия?)</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="103"/>
        <source>SDRplay service unreachable (is sdrplay_apiService running?)</source>
        <translation>Служба SDRplay недоступна (запущен ли sdrplay_apiService?)</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="121"/>
        <source>No SDRplay device found (OTG cable?)</source>
        <translation>Устройство SDRplay не найдено (кабель OTG?)</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="175"/>
        <source>RSPdx panadapter · IF %1 MHz · VFO %2 MHz</source>
        <translation>RSPdx панорама · ПЧ %1 MHz · VFO %2 MHz</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="177"/>
        <source>RSPdx receiving · %1 step(s)</source>
        <translation>RSPdx принимает · шагов: %1</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="188"/>
        <source>RSPdx stopped</source>
        <translation>RSPdx остановлен</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.cpp" line="337"/>
        <source>RSPdx was unplugged</source>
        <translation>RSPdx отключён</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.h" line="39"/>
        <source>FM 87.5–108</source>
        <translation>FM 87,5–108</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.h" line="40"/>
        <source>Airband 118–137</source>
        <translation>Авиадиапазон 118–137</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.h" line="43"/>
        <source>40 m (7.0–7.2)</source>
        <translation>40 м (7,0–7,2)</translation>
    </message>
    <message>
        <location filename="../src/sdrplaysource.h" line="44"/>
        <source>20 m (14.0–14.35)</source>
        <translation>20 м (14,0–14,35)</translation>
    </message>
</context>
<context>
    <name>SyntheticSource</name>
    <message>
        <location filename="../src/syntheticsource.cpp" line="8"/>
        <source>synthetic</source>
        <translation>синтетический</translation>
    </message>
    <message>
        <location filename="../src/syntheticsource.h" line="13"/>
        <source>Synthetic (demo)</source>
        <translation>Синтетический (демо)</translation>
    </message>
    <message>
        <location filename="../src/syntheticsource.h" line="18"/>
        <source>2.4 GHz</source>
        <translation>2,4 GHz</translation>
    </message>
</context>
</TS>
