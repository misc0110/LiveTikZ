#ifndef LIVETIKZ_H
#define LIVETIKZ_H

#include <KMenuBar>
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
#include <QTextEdit>
#include <QTextCursor>
#include <QTimer>
#include <kparts/mainwindow.h>
#include <ktexteditor/document.h>
#include <ktexteditor/view.h>
#include <poppler-qt4.h>
#include <ZoomScrollImage.h>

class MainWindow : public KParts::MainWindow {
  Q_OBJECT
public:
  MainWindow();
  virtual ~MainWindow();

public slots:
  void load(const KUrl &url);
  void load();
  void textInserted(KTextEditor::Document *document, const KTextEditor::Range &range);
  void textRemoved(KTextEditor::Document *document, const KTextEditor::Range &range);
  void refresh();
  void browse();
  void render(double scale);
  void renderFinished(int code);
  void showCompilerSelection();
  void updateLog();

private:
  void setupActions();
  void setupMenu();
  void setupUI();
  void setupEditor();

  void render();
  void compile(QTemporaryFile &file);
  
  void appendLog(QString str);


  QWidget *window;
  QSplitter *splitView;
  QHBoxLayout *mainLayout;
  QHBoxLayout *templateLayout;
  QHBoxLayout *logLayout;
  QSpacerItem *spacerItem;
  QVBoxLayout *leftLayout;

  KParts::ReadWritePart *katePart;
  KTextEditor::View *view;
  KTextEditor::Document *doc;
  ZoomScrollImage *display;
  Poppler::Document *currentDoc;

  QTimer *refreshTimer;
  QTimer *renderTimer;

  QTextEdit *log;

  KUrl templateFile;
  QLineEdit *templateLabel;
  QPushButton *browseButton;

  QPushButton* killButton;
  
  QProcess *renderProcess;
  QString renderOutput;
};

#endif
