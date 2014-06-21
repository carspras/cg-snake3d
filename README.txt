Visual Studio 10.0, oder neuer
  1.  Öffnen Sie das Projekt <unzip-dir>/aufgabe_2/aufgabe2.vcxproj
  2.  Klicken Sie die eventuell auftretenden (Konvertierungs-)Warnungen weg
  3.  Bauen Sie die Projektmappe (F7)
  4.  Führen Sie das Projekt aus (Strg+F5)

cmake
  1.  Laden Sie cmake herunter (http://www.cmake.org/cmake/resources/software.html)
      und installieren Sie es.
  2.  Starten Sie die cmake-gui
  3.  Geben Sie den Pfad zum  CMakeLists.txt (also: <unzip-dir>/aufgabe_2/) in 
      das "Where ist the source code"-Feld ein (!!!) Vorsicht mit Leerzeichen
      im Verzeichnisnamen, evtl. das Verzeichnis umbenennen
  4.  Geben Sie den _gleichen_ Pfad in das "Where to build the binaries"-Feld 
      ein.
  5.  Drucken Sie "Configure"
  6.  Wählen Sie Ihren Compiler/Ihre IDE aus
      (!!!)  Unter Mac OS X soll man die "XCode command line tools" installiert 
      haben, sonst findet cmake den Compiler nicht.  
  7.  Drucken Sie "Generate"
  8.  Gehen Sie zu <unzip-dir>/aufgabe_2/
  9.  Benutzen Sie die generierte Projekt-/Build-Datei

QtCreator
  1.  Mit dem QtCreator kann man auch cmake Projekte einlesen, s. 
      https://qt-project.org/doc/qtcreator-2.7/creator-project-cmake.html
      (!!!) Leerzeichen im Pfad machen dabei immer Probleme

Bemerkungen:
  -  Für die Vorlesung wird OpenGL 3.2 (besser 3.3+) vorausgesetzt.  
  -  Aktualisieren Sie Ihre Grafikkartentreiber; alle aktuellen Grafikkarten 
     (die letzte 4 Jahre) haben OpenGL 3.2+ Unterstützung.
  -  Frühere Versionen von Mac OS X unterstützen OpenGL nur bis Version 2.1, 
     OpenGL 3.X wird erst ab Mac OS X 10.8 Mountain Lion unterstützt
  -  OpenGL Extension Viewer ist ein nützliches Werkzeug um die Fähigkeiten 
     Ihrer Grafikkarten(-treiber) zu überprüfen

  -  Wenn man ein Projekt selbständig erstellen möchte, sollte man folgendes 
     beachten:
    -  Visual Studio benutzt "auto linking" für die notwendigen 
       Systembibliotheken, für alle anderen IDEs unter Windows
       (QtCreator, CodeBlocks, Dev-CPP usw.) sollte man glu32.lib und
       opengl32.lib (bzw. die entsprechenden *.a Dateien) zu den
       Linker-Optionen hinzufügen.
    -  Unter XCode sollte man die "automatic reference counting" ausschalten, 
       die Datei framework.cpp als Objective-C++ kompilieren und die
       OpenGL und Cocoa Frameworks zu den Linker-Optionen hinzufügen
    -  Unter Linux sollte man die CMakeLists.txt als Vorlage benutzen - das 
       FIND_OPENGL Packet macht einiges einfacher.  
    -  Mit QtCreator Unter Mac OS X sollte man die Datei framework.cpp auf 
       framework.mm umbenennen und als Objective-C++ (OBJECTIVE_SOURCES
       target in der *.pro Datei) kompilieren.
  