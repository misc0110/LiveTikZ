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
#include <string>
#include <thread>
#include <chrono>

#include <QtCore/QDebug>
#undef QT_NO_DEBUG
// #include <kdebug.h>

QString getFilePath(QTemporaryDir* dir, const char* file) {
  QString path(dir->path());
  path.append("/");
  path.append(file);
  return path;
}

void MainWindow::textChanged(KTextEditor::Document *document) {
  (void)document;
  refreshTimer->start(1000);
  watchme(texdir.path());
}

void MainWindow::documentSaved(KTextEditor::Document *document, bool saveas) {
  (void)document;
  (void)saveas;
  std::cout << "Document saved" << std::endl;
  usersaved = true;
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
  usersaved = false;
  QSettings settings;
  QString program = settings.value("compiler", "pdflatex").toString();

  QStringList arguments;
  QTemporaryDir tmpdir;
  tmpdir.setAutoRemove(true);
  
  if (program == "pdflatex") {
    arguments << "-halt-on-error";
  }
  if(!dir || dir->path() == "") {
    return;
  }
  if(texdir.absolutePath() == "") {
    std::cout << "need a temporary directory" << std::endl;
    texdir = QFileInfo(tmpdir.path());
  }
  QFileInfo templateDir = QFileInfo(templateFile.path());

  QString odir("--output-directory=");
  odir.append(dir->path());
  arguments << odir;

  arguments << getFilePath(dir, "_livetikz_preview.tex");

  std::cout << "Compiler arguments: " << arguments.join(QChar(' ')).toStdString() << std::endl;
  std::cout << "Compiler workdir: " << workdir.absolutePath().toStdString() << std::endl;

  renderProcess = new QProcess(this);
  renderProcess->setWorkingDirectory(workdir.absolutePath());
  clearLog();
  renderProcess->start(program, arguments);

  log->setText("Compiling...");
  renderOutput = "";
  connect(renderProcess, SIGNAL(finished(int)), this, SLOT(renderFinished(int)));
  //connect(renderProcess, SIGNAL(errorOccurred(QProcess::ProcessError)), this, SLOT(renderFailed(QProcess::ProcessError)));
  connect(renderProcess, SIGNAL(readyReadStandardError()), this, SLOT(updateLog()));
  connect(renderProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(updateLog()));
  connect(&templateWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(handleTemplateChanged(const QString&)));

  connect(killButton, SIGNAL(clicked()), renderProcess, SLOT(kill()));
  killButton->setVisible(true);
}

void MainWindow::refresh() {
  if(!dir) {
    dir = new QTemporaryDir();
    dir->setAutoRemove(false);
  }

  if (dir->isValid()) {
    //~ QFile file(dir->filePath("_livetikz_preview.tex"));
    QFile file(getFilePath(dir, "_livetikz_preview.tex"));

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
  } else if(dir) {
      std::cout << "Deleting invalid dir" << dir->path().toStdString() << std::endl;
      delete dir;
      dir = NULL;
  }
}

void MainWindow::clearLog() {
  renderOutput = QString();
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
    appendLog(renderProcess->readAllStandardError());
  }
}

void MainWindow::renderFailed(QProcess::ProcessError) {
    appendLog("Failed to execute compiler\n");
    renderFinished(1);
}


void MainWindow::renderFinished(int code) {
  if (code == 0) {
    appendLog("Done!\n");
  } else {
    appendLog("Error!\n");
  }

  killButton->setVisible(false);
  
  if(!dir) return;
  
  delete currentDoc;
  currentDoc = NULL;

  //~ QFile pdf_file(QDir::cleanPath(getFilePworkdir.absolutePath() + QDir::separator() + "_livetikz_preview.pdf"));
  //~ if (pdf_file.exists()) {
    //~ QFile::rename(QDir::cleanPath(texdir.absolutePath() + QDir::separator() + "_livetikz_preview.pdf"), dir->filePath("_livetikz_preview.pdf"));
    //~ QFile::rename(QDir::cleanPath(workdir.absolutePath() + QDir::separator() + "_livetikz_preview.pdf"), getFilePath(dir, "_livetikz_preview.pdf"));
    //~ currentDoc = Poppler::Document::load(dir->filePath("_livetikz_preview.pdf"));
    QFile::remove(getFilePath(dir, "livetikz_preview.pdf"));
    QFile::rename(getFilePath(dir, "_livetikz_preview.pdf"), getFilePath(dir, "livetikz_preview.pdf"));
    
    currentDoc = Poppler::Document::load(getFilePath(dir, "livetikz_preview.pdf"));
    if (currentDoc) {
      currentDoc->setRenderHint(Poppler::Document::TextAntialiasing);
      currentDoc->setRenderHint(Poppler::Document::Antialiasing);
      currentDoc->setRenderHint(Poppler::Document::TextHinting);
      currentDoc->setRenderHint(Poppler::Document::TextSlightHinting);
      currentDoc->setRenderHint(Poppler::Document::ThinLineSolid);
      render();
    }
  //~ }

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
  updateTemplate(templateFile.path());
  templateLabel->setText(templateFile.url(QUrl::PreferLocalFile));
  
  texfileWatcher = NULL;

  doc->setHighlightingMode("Latex");

  refreshTimer = new QTimer(this);
  refreshTimer->setSingleShot(true);
  connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refresh()));
  refreshTimer->start(1000);

  connect(display, SIGNAL(zoomChanged(double)), this, SLOT(render(double)));
  connect(browseButton, SIGNAL(clicked()), SLOT(browse()));
  connect(workdirButton, SIGNAL(clicked()), SLOT(chooseWorkdir()));
  
  connect(prevImage, SIGNAL(triggered()), this, SLOT(gotoPreviousImage()));
  connect(nextImage, SIGNAL(triggered()), this, SLOT(gotoNextImage()));

  connect((QObject *)doc, SIGNAL(textChanged(KTextEditor::Document *)), this,
          SLOT(textChanged(KTextEditor::Document *)));
  connect(templateLabel, SIGNAL(textEdited(const QString&)), this, 
          SLOT(updateTemplate(const QString&)));
  connect((QObject *)doc, SIGNAL(documentSavedOrUploaded(KTextEditor::Document *, bool)), this,
          SLOT(documentSaved(KTextEditor::Document *, bool)));
}

void MainWindow::showCompilerSelection() {
  QStringList items;
  items << tr("pdflatex") << tr("latexrun") << tr("xelatex") << tr("lualatex");

  bool ok;
  QString item = QInputDialog::getItem(this, tr("LaTeX compiler"), tr("Compiler:"), items, 0, false, &ok);
  if (ok && !item.isEmpty()) {
    QSettings settings;
    settings.setValue("compiler", item);
  }
}

MainWindow::~MainWindow() {
  /* remove temp dir */
  if (dir) {
    dir->remove();
    delete dir;
    dir = NULL;
  }
  if (texfileWatcher) {
    delete texfileWatcher;
    texfileWatcher = NULL;
  }
}

void MainWindow::watchme(const QString& filename) {
  if (texfileWatcher) {
    /* We're already watching */
    return;
  }
  /* Some editors delete and re-create files on save.
   * This somehow breaks our watcher. On re-watching, the event is
   * fired twice, then three times ...
   * To avoid this, completely destroy and re-connect the watcher.
   */
  texfileWatcher = new QFileSystemWatcher();
  connect(texfileWatcher, SIGNAL(fileChanged(const QString&)), this, SLOT(handleTexfileChanged(const QString&)));

  std::cout << "Watching " << filename.toStdString() << std::endl;
  int retry_count = 10;
  while(!texfileWatcher->addPath(filename) && retry_count--) {
    /* If the file has not been re-created yet, wait some time */
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Retrying to watch..." << std::endl;
  }
}

void MainWindow::load(const QUrl &url) { 
  std::cout << "Loading " << url.toString().toStdString() << std::endl;
  texdir = QFileInfo(url.toLocalFile());
  watchme(url.toLocalFile());
  
  workdir = texdir;
  updateRootDirectory();
  katePart->openUrl(url); 
}

void MainWindow::setupActions() {
  KStandardAction::open(this, SLOT(load()), actionCollection());
  //KStandardAction::save(this, SLOT(save()), actionCollection());
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
  
  workdirLabel = new QLineEdit("Working directory");
  workdirLabel->setReadOnly(true);
  workdirButton = new QPushButton("Change working directory...");
  
  workdirLayout = new QHBoxLayout;
  workdirLayout->addWidget(workdirLabel);
  workdirLayout->addWidget(workdirButton);

  containerLayout = new QVBoxLayout;
  splitLogView = new QSplitter(Qt::Vertical, window);
  
  wdtmplLayout = new QHBoxLayout;
  wdtmplLayout->addLayout(templateLayout);
  wdtmplLayout->addLayout(workdirLayout);
  containerLayout->addLayout(wdtmplLayout);
  
  splitView->addWidget(view);

  display = new ZoomScrollImage;

  log = new QTextEdit(window);
  log->setReadOnly(true);
  log->ensureCursorVisible();
  QFontMetrics m(log->font());
//   log->setFixedHeight(5 * m.lineSpacing());

  logLayout = new QHBoxLayout;
  
  splitView->addWidget(display);
  splitLogView->addWidget(splitView);
  
  logLayout->addWidget(log);
  
  killButton = new QPushButton("Abort compilation");
  killButton->setVisible(false);
  logLayout->addWidget(killButton);
  
  logWidget = new QWidget();
  logWidget->setLayout(logLayout);
  
  splitLogView->addWidget(logWidget);
  splitLogView->setStretchFactor(0, 9);
  splitLogView->setStretchFactor(1, 1);
  
  containerLayout->addWidget(splitLogView);
  
  mainLayout->addLayout(containerLayout);

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

void MainWindow::handleTemplateChanged(const QString& filename) {
  std::cout << filename.toStdString() << " changed." << std::endl;
  updateTemplate(filename);
}

void MainWindow::handleTexfileChanged(const QString& filename) {
  if (!usersaved) {
    std::cout << "Texfile " << filename.toStdString() << " externally changed." << std::endl;
    /* Reloading */
    katePart->openUrl(QUrl::fromLocalFile(filename));
  }
  /* Always re-watch file */
  if (texfileWatcher) {
    delete texfileWatcher;
    texfileWatcher = NULL;
  }
  watchme(filename);
}

void MainWindow::updateTemplate(const QString& filename) {
  QSettings settings;
  QUrl oldTemplateFile = QUrl(settings.value("template", "").toString());
  templateWatcher.removePath(oldTemplateFile.path());

  templateFile = QUrl::fromLocalFile(filename);
  std::cout << "Loading template " << templateFile.path().toStdString() << std::endl;
  templateWatcher.addPath(templateFile.path());
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

void MainWindow::chooseWorkdir() {
    workdir = QFileInfo(QFileDialog::getExistingDirectory(this, QString("Select Working Directory"),
                                                workdir.absolutePath(), QFileDialog::ShowDirsOnly) + "/");
    std::cout << "Selected working directory: " << workdir.path().toStdString() << std::endl;
    updateRootDirectory(); 
    refresh();
}

void MainWindow::updateRootDirectory() {
    workdirLabel->setText(workdir.absolutePath());
}
