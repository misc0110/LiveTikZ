#include "mainwindow.h"

#include <KActionCollection>
#include <QFileDialog>
#include <KMessageBox>
#include <KService>
#include <KStandardAction>
#include <QUrl>
#include <KToolBar>

#include <QApplication>
#include <QTemporaryDir>
#include <QTemporaryFile>
#include <QFile>
#include <iostream>

#include <QtCore/QDebug>
#undef QT_NO_DEBUG
#include <kdebug.h>


void MainWindow::textChanged(KTextEditor::Document *document) {
  (void)document;
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

void MainWindow::compile() {
  QSettings settings;
  QString program = settings.value("compiler", "pdflatex").toString();
  QStringList arguments;
  QTemporaryDir tmpdir;
  tmpdir.setAutoRemove(true);
  
  if (program == "pdflatex") {
    arguments << "-halt-on-error";
  }
  if(dir->path() == "") {
    return;
  }
  if(texdir.absolutePath() == "") {
    std::cout << "need a temporary directory" << std::endl;
    texdir = QFileInfo(tmpdir.path());
  }
  
  arguments << dir->filePath("_livetikz_preview.tex");

  std::cout << "Arguments: " << dir->filePath("_livetikz_preview.tex").toStdString() << std::endl;
  std::cout << "WD: " << texdir.absolutePath().toStdString() << std::endl;
  
  renderProcess = new QProcess(this);
  renderProcess->setWorkingDirectory(texdir.absolutePath());
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
  if(!dir) {
    dir = new QTemporaryDir();
    dir->setAutoRemove(false);
  }

  if (dir->isValid()) {
    QFile file(dir->filePath("_livetikz_preview.tex"));

    if (file.open(QIODevice::ReadWrite | QIODevice::Text)) {
      QTextStream out(&file);

      QFile inputFile(templateFile.url(QUrl::PreferLocalFile));
      if (inputFile.exists() && inputFile.open(QIODevice::ReadOnly)) {
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

      out.flush();
      file.flush();
      file.close();

      if (renderProcess != NULL) {
        refreshTimer->start(1000); // wait for old rendering to finish
      } else {
        compile();
      }
    }
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
  
  if(!dir) return;
  
  delete currentDoc;
  currentDoc = NULL;

  QFile pdf_file(QDir::cleanPath(texdir.absolutePath() + QDir::separator() + "_livetikz_preview.pdf"));
  if (pdf_file.exists()) {
    QFile::rename(QDir::cleanPath(texdir.absolutePath() + QDir::separator() + "_livetikz_preview.pdf"), dir->filePath("_livetikz_preview.pdf"));
    currentDoc = Poppler::Document::load(dir->filePath("_livetikz_preview.pdf"));
    if (currentDoc) {
      currentDoc->setRenderHint(Poppler::Document::TextAntialiasing);
      currentDoc->setRenderHint(Poppler::Document::Antialiasing);
      currentDoc->setRenderHint(Poppler::Document::TextHinting);
      currentDoc->setRenderHint(Poppler::Document::TextSlightHinting);
      currentDoc->setRenderHint(Poppler::Document::ThinLineSolid);

      render();
    }
  }

  dir->remove();
  delete dir;
  dir = NULL;

  renderProcess = NULL;
}

MainWindow::MainWindow() : currentDoc(NULL), renderProcess(NULL), currentPage(0) {
  QSettings settings;

  setupEditor();
  setupActions();
  setupMenu();
  setupUI();

  dir = NULL;
  templateFile = QUrl(settings.value("template", "").toString());
  templateLabel->setText(templateFile.url(QUrl::PreferLocalFile));

  doc->setHighlightingMode("Latex");

  refreshTimer = new QTimer(this);
  refreshTimer->setSingleShot(true);
  connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
  refreshTimer->start(1000);

  connect(display, SIGNAL(zoomChanged(double)), this, SLOT(render(double)));
  connect(browseButton, SIGNAL(clicked()), SLOT(browse()));
  
  connect(prevImage, SIGNAL(triggered()), this, SLOT(gotoPreviousImage()));
  connect(nextImage, SIGNAL(triggered()), this, SLOT(gotoNextImage()));

  connect((QObject *)doc, SIGNAL(textChanged(KTextEditor::Document *)), this,
          SLOT(textChanged(KTextEditor::Document *)));
  connect(templateLabel, SIGNAL(textEdited(const QString&)), this, SLOT(updateTemplate(const QString&)));
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

void MainWindow::load(const QUrl &url) { 
    texdir = QFileInfo(url.toLocalFile());
    katePart->openUrl(url); 
}

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
  
  setWindowIcon(QIcon(":/logo.png"));
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

void MainWindow::load() { load(QFileDialog::getOpenFileUrl()); }

void MainWindow::updateTemplate(const QString& filename) {
  QSettings settings;
  templateFile = QUrl::fromUserInput(filename);
  settings.setValue("template", filename);
  refresh();
}

void MainWindow::browse() {
  QUrl newTemplateFile = QFileDialog::getOpenFileUrl(this, QString("Open template"));
  if (newTemplateFile.fileName() != "") {
      QString filename = newTemplateFile.url(QUrl::PreferLocalFile);
      templateLabel->setText(filename);
      updateTemplate(filename);
  }
}
