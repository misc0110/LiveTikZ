#ifndef LIVETIKZ_H
#define LIVETIKZ_H

#include <QMenuBar>
#include <QDesktopWidget>
#include <QFileDialog>
#include <QScrollBar>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QScrollArea>
#include <QSettings>
#include <QSplitter>
#include <QTemporaryFile>
#include <QTemporaryDir>
#include <QFileSystemWatcher>
#include <QTextEdit>
#include <QTextCursor>
#include <QTimer>
#include <QMessageLogger>
#include <KParts/MainWindow>
#include <KTextEditor/Document>
#include <KTextEditor/View>
#include <poppler-qt5.h>
#include <ZoomScrollImage.h>


class MainWindow : public KParts::MainWindow {
  Q_OBJECT
public:
  MainWindow();
  virtual ~MainWindow();

public slots:
  void load(const QUrl &url);
  void load();
  void textChanged(KTextEditor::Document *document);
  void documentSaved(KTextEditor::Document *document, bool saveas);
  void refresh();
  void browse();
  void chooseWorkdir();
  void render(double scale);
  void renderFinished(int code);
  void renderFailed(QProcess::ProcessError);
  void showCompilerSelection();
  void updateLog();
  void watchme(const QString& filename);
  void handleTemplateChanged(const QString& filename);
  void handleTexfileChanged(const QString& filename);
  void gotoPreviousImage();
  void gotoNextImage();
  void updateTemplate(const QString& filename);

private:
  void setupActions();
  void setupMenu();
  void setupUI();
  void setupEditor();

  void render();
  void compile();

  void clearLog();
  void appendLog(QString str);
  
  void updateRootDirectory();

  QWidget *window;
  QSplitter *splitView;
  QHBoxLayout *mainLayout;
  QHBoxLayout *templateLayout;
  QHBoxLayout *workdirLayout;
  QHBoxLayout *logLayout;
  QSpacerItem *spacerItem;
  QVBoxLayout *containerLayout;
  QHBoxLayout *wdtmplLayout;
  QSplitter* splitLogView;
  QWidget *logWidget;

  KParts::ReadWritePart *katePart;
  KTextEditor::View *view;
  KTextEditor::Document *doc;
  ZoomScrollImage *display;
  QFileInfo texdir;
  QFileInfo workdir;
  QTemporaryDir* dir;
  Poppler::Document *currentDoc;

  QTimer *refreshTimer;
  QTimer *renderTimer;

  QTextEdit *log;

  QUrl templateFile;
  QLineEdit *templateLabel;
  QPushButton *browseButton;
  
  QLineEdit *workdirLabel;
  QPushButton *workdirButton;

  QPushButton* killButton;

  QFileSystemWatcher templateWatcher;
  QFileSystemWatcher *texfileWatcher;

  QProcess *renderProcess;
  QString renderOutput;
  
  QAction* nextImage;
  QAction* prevImage;
  
  int currentPage;
  bool usersaved;
};

#endif
