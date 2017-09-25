#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KUrl>

#include "mainwindow.h"

int main(int argc, char *argv[]) {
  KAboutData aboutData("livetikz", "livetikz", ki18n("livetikz"), "0.1", ki18n("A tikz editor with live preview."),
                       KAboutData::License_GPL, ki18n("Copyright (c) 2017 Michael Schwarz"));
  KCmdLineArgs::init(argc, argv, &aboutData);

  KCmdLineOptions options;
  options.add("+[file]", ki18n("Document to open"));
  KCmdLineArgs::addCmdLineOptions(options);

  KApplication app;

  MainWindow *window = new MainWindow();
  window->show();

  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  if (args->count()) {
    window->load(args->url(0).url());
  }

  return app.exec();
}
