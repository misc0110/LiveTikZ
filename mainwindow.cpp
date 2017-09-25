#include "mainwindow.h"

#include <kaction.h>
#include <kactioncollection.h>
#include <kconfig.h>
#include <kedittoolbar.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kservice.h>
#include <kstandardaction.h>
#include <kstatusbar.h>
#include <kurl.h>
#include <ktoolbar.h>

#include <QApplication>
#include <iostream>

#include <QtCore/QDebug>
#undef QT_NO_DEBUG
#include <kdebug.h>

void MainWindow::textInserted(KTextEditor::Document *document, const KTextEditor::Range &range) {
  (void)document;
  (void)range;
  refreshTimer->start(1000);
}

void MainWindow::textRemoved(KTextEditor::Document *document, const KTextEditor::Range &range) {
  (void)document;
  (void)range;
  refreshTimer->start(1000);
}

void MainWindow::render() { render(display->getScale()); }

void MainWindow::gotoNextImage() {
  if(currentDoc && currentPage < currentDoc->numPages()) {
    currentPage++;
    render();
  }
}

void MainWindow::gotoPreviousImage() {
  if(currentPage > 0) {
    currentPage--;
    render();
  }
}

void MainWindow::render(double scale) {
  if (currentDoc && display) {
    if(currentPage >= currentDoc->numPages()) {
        currentPage = currentDoc->numPages() - 1;
    }
    if(currentPage < 0) {
        currentPage = 0;
    }
    prevImage->setVisible(currentDoc->numPages() > 1);
    nextImage->setVisible(currentDoc->numPages() > 1);
    
    QImage image = currentDoc->page(currentPage)->renderToImage(scale * physicalDpiX(), scale * physicalDpiY());
    display->setImage(image);
  }
}

void MainWindow::compile(QTemporaryFile &file) {
  QSettings settings;
  QString program = settings.value("compiler", "pdflatex").toString();
  QStringList arguments;
  if (program == "pdflatex") {
    arguments << "-halt-on-error";
  }
  arguments << file.fileName();

  renderProcess = new QProcess(this);
  renderProcess->start(program, arguments);

  log->setText("Compiling...");
  renderOutput = "";
  connect(renderProcess, SIGNAL(finished(int)), this, SLOT(renderFinished(int)));

  connect(renderProcess, SIGNAL(readyReadStandardError()), this, SLOT(updateLog()));
  connect(renderProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(updateLog()));
  
  connect(killButton, SIGNAL(clicked()), renderProcess, SLOT(kill()));
  killButton->setVisible(true);
}

void MainWindow::refresh() {
  QTemporaryFile file;

  file.setAutoRemove(false);
  if (file.open()) {
    QTextStream out(&file);

    QFile inputFile(templateFile.pathOrUrl());
    if (inputFile.open(QIODevice::ReadOnly)) {
      QTextStream in(&inputFile);
      while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed() == "<>") {
          out << doc->text() << "\n";
        } else {
          out << line << "\n";
        }
      }
      inputFile.close();
    } else {
      out << "\\documentclass{article}\n"
          << "\\usepackage{tikz}\n"
          << "\\usepackage{color}\n"
          << "\\usepackage{amssymb}\n"
          << "\\usepackage{pgfplots}\n"
          << "\\usetikzlibrary{pgfplots.groupplots}\n"
          << "\\usetikzlibrary{arrows}\n"
          << "\\usetikzlibrary{patterns}\n"
          << "\\usetikzlibrary{positioning}\n"
          << "\\usetikzlibrary{decorations.pathreplacing}\n"
          << "\\usetikzlibrary{shapes.arrows}\n"
          << "\\usetikzlibrary{pgfplots.groupplots}\n"
          << "\\pgfplotsset{compat=1.13}\n";
      out << "\\begin{document}\n";
      out << doc->text() << "\n";
      out << "\\end{document}\n";
    }

    file.flush();
    file.close();

    if (renderProcess != NULL) {
      refreshTimer->start(1000); // wait for old rendering to finish
    }

    compile(file);
  }
}

void MainWindow::appendLog(QString str) {
  renderOutput += str;
  log->setText(renderOutput);
  log->verticalScrollBar()->setValue(log->verticalScrollBar()->maximum());
  QTextCursor c = log->textCursor();
  c.movePosition(QTextCursor::End);
  log->setTextCursor(c);
}

void MainWindow::updateLog() {
  if (renderProcess) {
    appendLog(renderProcess->readAllStandardOutput());
  }
}

void MainWindow::renderFinished(int code) {
  if (code == 0) {
    appendLog("Done!");
  } else {
    appendLog("Error!");
  }

  killButton->setVisible(false);
  
  delete currentDoc;
  currentDoc = Poppler::Document::load("qt_temp.pdf");
  if (currentDoc) {
    currentDoc->setRenderHint(Poppler::Document::TextAntialiasing);
    currentDoc->setRenderHint(Poppler::Document::Antialiasing);
    currentDoc->setRenderHint(Poppler::Document::TextHinting);
    currentDoc->setRenderHint(Poppler::Document::TextSlightHinting);
    currentDoc->setRenderHint(Poppler::Document::ThinLineSolid);

    render();
  }
  renderProcess = NULL;
}

MainWindow::MainWindow() : currentDoc(NULL), renderProcess(NULL), currentPage(0) {
  QCoreApplication::setOrganizationName("misc0110");
  QCoreApplication::setOrganizationDomain("misc0110.net");
  QCoreApplication::setApplicationName("livetikz");
  QSettings settings;

  setupEditor();
  setupActions();
  setupMenu();
  setupUI();

  templateFile = KUrl(settings.value("template", "").toString());
  if (templateFile != "") {
    templateLabel->setText(templateFile.pathOrUrl());
  }

  doc->setHighlightingMode("Latex");

  refreshTimer = new QTimer(this);
  refreshTimer->setSingleShot(true);
  connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
  refreshTimer->start(1000);

  connect(display, SIGNAL(zoomChanged(double)), this, SLOT(render(double)));
  connect(browseButton, SIGNAL(clicked()), SLOT(browse()));
  
  connect(prevImage, SIGNAL(triggered()), this, SLOT(gotoPreviousImage()));
  connect(nextImage, SIGNAL(triggered()), this, SLOT(gotoNextImage()));

  connect((QObject *)doc, SIGNAL(textInserted(KTextEditor::Document *, KTextEditor::Range)), this,
          SLOT(textInserted(KTextEditor::Document *, KTextEditor::Range)));
  connect((QObject *)doc, SIGNAL(textRemoved(KTextEditor::Document *, KTextEditor::Range)), this,
          SLOT(textRemoved(KTextEditor::Document *, KTextEditor::Range)));
}

void MainWindow::showCompilerSelection() {
  QStringList items;
  items << tr("pdflatex") << tr("latexrun");

  bool ok;
  QString item = QInputDialog::getItem(this, tr("LaTeX compiler"), tr("Compiler:"), items, 0, false, &ok);
  if (ok && !item.isEmpty()) {
    QSettings settings;
    settings.setValue("compiler", item);
  }
}

MainWindow::~MainWindow() {}

void MainWindow::load(const KUrl &url) { katePart->openUrl(url); }

void MainWindow::setupActions() {
  KStandardAction::open(this, SLOT(load()), actionCollection());
  KStandardAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());
}

void MainWindow::setupMenu() {
  QMenu *tikzMenu = new QMenu(tr("LiveTikZ"), this);
  QAction *compiler = tikzMenu->addAction(tr("&Compiler"));
  connect(compiler, SIGNAL(triggered()), this, SLOT(showCompilerSelection()));
  menuBar()->addMenu(tikzMenu);
}

void MainWindow::setupUI() {
  window = new QWidget;
  mainLayout = new QHBoxLayout;
  splitView = new QSplitter(window);
  templateLabel = new QLineEdit("Select template...");

  browseButton = new QPushButton("Browse");
  templateLayout = new QHBoxLayout;
  templateLayout->addWidget(templateLabel);
  templateLayout->addWidget(browseButton);

  leftLayout = new QVBoxLayout;
  leftLayout->addLayout(templateLayout);
  splitView->addWidget(view);

  display = new ZoomScrollImage;

  log = new QTextEdit(window);
  log->setReadOnly(true);
  log->ensureCursorVisible();
  QFontMetrics m(log->font());
  log->setFixedHeight(5 * m.lineSpacing());

  logLayout = new QHBoxLayout;
  
  splitView->addWidget(display);
  leftLayout->addWidget(splitView);
  
  logLayout->addWidget(log);
  
  killButton = new QPushButton("Kill pdflatex");
  killButton->setVisible(false);
  logLayout->addWidget(killButton);
  
  leftLayout->addLayout(logLayout);
  
  mainLayout->addLayout(leftLayout);

  window->setLayout(mainLayout);
  setCentralWidget(window);
  setupGUI(ToolBar | Keys | StatusBar | Save);
  createGUI(katePart);
  
  toolBar()->addSeparator();
  prevImage = toolBar()->addAction(QIcon::fromTheme("go-previous"), "Previous image");
  nextImage = toolBar()->addAction(QIcon::fromTheme("go-next"), "Next image");
  prevImage->setVisible(false);
  nextImage->setVisible(false);

  QDesktopWidget widget;
  QRect mainScreenSize = widget.availableGeometry(widget.primaryScreen());
  this->resize(mainScreenSize.width() * 0.7, mainScreenSize.height() * 0.7);
  log->resize(mainScreenSize.width() * 0.7, mainScreenSize.height() * 0.7 * 0.1);
  display->resize(mainScreenSize.width() * 0.7 * 0.5, mainScreenSize.height() * 0.7 * 1);
  view->resize(mainScreenSize.width() * 0.7 * 0.5, mainScreenSize.height() * 0.7 * 0.9);
}

void MainWindow::setupEditor() {

  KService::Ptr service = KService::serviceByDesktopName("katepart");
  if (service) {
    katePart = service->createInstance<KParts::ReadWritePart>(0);
    if (katePart) {
      view = ((KTextEditor::View *)katePart->widget());
      doc = view->document();
    } else {
      KMessageBox::error(this, "Could not create editor");
      qApp->quit();
    }
  } else {
    KMessageBox::error(this, "Service katepart not found - please install kate");
    qApp->quit();
  }
}

void MainWindow::load() { load(KFileDialog::getOpenUrl()); }

void MainWindow::browse() {
  KUrl newTemplateFile = KFileDialog::getOpenUrl();
  if (newTemplateFile.pathOrUrl() != "") {
    templateFile = newTemplateFile;
    templateLabel->setText(templateFile.pathOrUrl());
    QSettings settings;
    settings.setValue("template", templateFile.pathOrUrl());
  }
}
