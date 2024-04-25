# Geography Learner App
Es gibt verschiedene Varianten, sich auf Toppografieprüfungen vorzubereiten und diese Applikation ist eine Option, dies zu tun.
Die Geography-Learner app wurde spezifisch kreiert, um die Grundzüge der Welttoppografie zu lernen und wiederholen.
[Direkt zu den Downloads](#Download)
## Aufgabentypen
Unterstützt werden die "typischen" Toppografiefragen, bei denen ein Punkt gegeben ist und der Name der Ortschaft, der Insel,
des Meeres, des Kontinentes, des Flusses oder des Gebirges eingegeben werden müssen.
Zudem gibt es die Möglichkeit mit vortgeschrittenen Fragen zu lernen, bei denen mehrere Informationen kombiniert werden müssen.
So kann beispielsweise gefragt werden, wohin der Mississippi fliesst, oder in welchem Land Lima liegt.

[![Geography Learner](PreviewImage.png)](PreviewImage.png)

## Antworten
Die akzeptierten Antworten sind auf deutsch, und oft werden lokale Varianten ebenfalls akzeptiert.
Im Falle eines Namens mit sonderzeichen wie "é", "à", "ã", etc. werden sowohl die Varianten ohne Akzent ("e", "a", "a"),
als auch die Varianten mit vorgesetzten Akzenten ("´e", "`a", "~a") akzeptiert,
da aufgrunde von limitationen des verwendeten Grafik-Frameworks die normale Schreibform solcher Sonderzeichen nicht akzeptiert wird.

## Download
Unterstützte Plattformen:
* [Windows](Installer/Geography-Learner.msi)
  
<details><summary>Kann der GeographyLearner auch für Mac, IOS oder Android gebraucht werden?</summary>


  Der GeographyLearner ist __zurzeit leider nur für Windows__ direkt als Downloadlink erhältlich.
  
  Grundsätzlich ist die Antwort aber *ja*.
  Die Ressourcen sind auf dieser Seite vorhanden und können für alle dieser Plattformen *lokal kompilliert* werden.

  <details><summary><h3>Lokal kompilieren</h3></summary>
  
1. Lade das vollständige `GeographyLearner` Repository herunter 
2. Lade den [Qt Creator](https://www.qt.io/download) auf das zu verwendende Gerät herunter & installiere die Applikation (die Standardinstallation reicht aus)
3. Wähle eines der folgenden:
   * Grafische Installation
      1. Öffne die Qt Creator app
      2. Öffne ein existierendes Projekt, indem du die `CMakeLists.txt` datei aus dem Repository auswählst
      3. Konfiguriere das Projekt, indem du nur das `Release`-Target auswählst
      4. Im `Build`-Menu wähle `Build Project`
      5. Die ausführbare Datei befindet sich nun unter 'build/\[Compilername\]/GeographyLearner'
   * Installation über die Command Line (NICHT EMPFOHLEN)
      1. Öffne ein terminal, das Zugang zur Qt installation und dem zur Installation passenden C++ Compiler hat
      2. Versichere dich, das die Buildtools CMake und Ninja installiert sind
      3. Wechsle in den `GeographyLearner`-Ordner 
      4. Führe `cmake -S. -Bbuild -G Ninja` aus
      5. Führe `ninja -C build` aus
      6. Die ausführbare Datei befindet sich nun unter 'build/GeographyLearner'

</details>
  
</details>

## Verbesserungen
Sollte eine gebräuchliche Antwort nicht akzeptiert sein, oder ein anderer Fehler gefunden werden,
bin ich froh, wenn ein Fehlerreport in Github erstellt wird, sodass ich ihn korrigieren kann.
