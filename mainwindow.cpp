 /*
 *  This file is part of the KDE Help Center
 *
 *  Copyright (C) 1999 Matthias Elter (me@kde.org)
 *                2001 Stephan Kulow (coolo@kde.org)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <stdlib.h>
#include <assert.h>

#include <qsplitter.h>
#include <qtextedit.h>
#include <qlayout.h>

#include <kapplication.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <kiconloader.h>
#include <kmimemagic.h>
#include <krun.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <khtmlview.h>
#include <khtml_settings.h>
#include <kaction.h>
#include <kstatusbar.h>
#include <kstdaccel.h>
#include <kdialogbase.h>

#include "history.h"
#include "view.h"
#include "searchengine.h"
#include "fontdialog.h"
#include <kkeydialog.h>
using namespace KHC;

#include "mainwindow.h"
#include "mainwindow.moc"

class LogDialog : public KDialogBase
{
  public:
    LogDialog( QWidget *parent = 0 )
      : KDialogBase( Plain, i18n("Search Error Log"), Ok, Ok, parent, 0,
                     false )
    {
      QFrame *topFrame = plainPage();

      QBoxLayout *topLayout = new QVBoxLayout( topFrame );

      mTextView = new QTextEdit( topFrame );
      mTextView->setTextFormat( LogText );
      topLayout->addWidget( mTextView );

      resize( 600, 400 );
    }

    void setLog( const QString &log )
    {
      mTextView->setText( log );
    }

  private:
    QTextEdit *mTextView;
};


MainWindow::MainWindow(const KURL &url)
    : KMainWindow(0, "MainWindow"), DCOPObject( "KHelpCenterIface" ),
      mLogDialog( 0 )
{
    QSplitter *splitter = new QSplitter(this);

    mDoc = new View( splitter, 0, this, 0, KHTMLPart::DefaultGUI );
    connect( mDoc, SIGNAL( setWindowCaption( const QString & ) ),
             SLOT( setCaption( const QString & ) ) );
    connect( mDoc, SIGNAL( setStatusBarText( const QString & ) ),
             SLOT( statusBarMessage( const QString & ) ) );
    connect( mDoc, SIGNAL( onURL( const QString & ) ),
             SLOT( statusBarMessage( const QString & ) ) );
    connect( mDoc, SIGNAL( started( KIO::Job * ) ),
             SLOT( slotStarted( KIO::Job * ) ) );
    connect( mDoc, SIGNAL( completed() ),
             SLOT( documentCompleted() ) );
    connect( mDoc, SIGNAL( searchResultCacheAvailable() ),
             SLOT( enableLastSearchAction() ) );

    statusBar()->insertItem(i18n("Preparing Index"), 0, 1);
    statusBar()->setItemAlignment(0, AlignLeft | AlignVCenter);

    connect(mDoc->browserExtension(),
            SIGNAL(openURLRequest( const KURL &,
                                   const KParts::URLArgs &)),
            SLOT(slotOpenURLRequest( const KURL &,
                                     const KParts::URLArgs &)));

    mNavigator = new Navigator( mDoc, splitter, "nav");
    connect(mNavigator, SIGNAL(itemSelected(const QString &)),
            SLOT(slotOpenURL(const QString &)));
    connect(mNavigator, SIGNAL(glossSelected(const GlossaryEntry &)),
            SLOT(slotGlossSelected(const GlossaryEntry &)));

    splitter->moveToFirst(mNavigator);
    splitter->setResizeMode(mNavigator, QSplitter::KeepSize);
    setCentralWidget( splitter );
    QValueList<int> sizes;
    sizes << 220 << 580;
    splitter->setSizes(sizes);
    setGeometry(366, 0, 800, 600);

    KConfig *cfg = kapp->config();
    {
      KConfigGroupSaver groupSaver( cfg, "General" );
      if ( cfg->readBoolEntry( "UseKonqSettings", true ) ) {
        KConfig konqCfg( "konquerorrc" );
        const_cast<KHTMLSettings *>( mDoc->settings() )->init( &konqCfg );
      }
      const int zoomFactor = cfg->readNumEntry( "Font zoom factor", 100 );
      mDoc->setZoomFactor( zoomFactor );
    }

    setupActions();

    actionCollection()->addDocCollection( mDoc->actionCollection() );

    createGUI( "khelpcenterui.rc" );

    History::self().installMenuBarHook( this );

    connect( &History::self(), SIGNAL( goInternalUrl( const KURL & ) ),
             SLOT( goInternalUrl( const KURL & ) ) );

    if ( !url.isEmpty() ) slotOpenURL( url.url() );

    statusBarMessage(i18n("Ready"));

    const QRect rect = kapp->desktop()->availableGeometry( this );
    setMaximumSize( rect.width(), rect.height() );
}

MainWindow::~MainWindow()
{
}

void MainWindow::saveProperties( KConfig *config )
{
    kdDebug()<<"void MainWindow::saveProperties( KConfig *config )\n";
    config->setDesktopGroup();
    config->writePathEntry( "URL" , mDoc->baseURL().url() );
}

void MainWindow::readProperties( KConfig *config )
{
#if 0 // I don't understand it doesn't read in good group ...
    kdDebug()<<"void MainWindow::readProperties( KConfig *config )\n";
    config->setDesktopGroup();
    mDoc->slotReload( config->readPathEntry( "URL" ) );
#endif
}

void MainWindow::setupActions()
{
    KStdAction::quit( this, SLOT( close() ), actionCollection() );
    KStdAction::print( this, SLOT( print() ), actionCollection(),
                       "printFrame" );

    KStdAction::home( this, SLOT( slotShowHome() ), actionCollection() );
    mLastSearchAction = new KAction( i18n("&Last Search Result"), 0, this,
                                     SLOT( slotLastSearch() ),
                                     actionCollection(), "lastsearch" );
    mLastSearchAction->setEnabled( false );

    KStdAction::preferences( mNavigator, SLOT( showPreferencesDialog() ),
                             actionCollection() );
    KStdAction::keyBindings( this, SLOT( slotConfigureKeys() ), actionCollection() );

    KConfig *cfg = KGlobal::config();
    cfg->setGroup( "Debug" );
    if ( cfg->readBoolEntry( "SearchErrorLog", false ) ) {
      new KAction( i18n("Show Search Error Log"), 0, this,
                   SLOT( showSearchStderr() ), actionCollection(),
                   "show_search_stderr" );
    }

    History::self().setupActions( actionCollection() );

    new KAction( i18n( "Configure Fonts..." ), KShortcut(), this, SLOT( slotConfigureFonts() ), actionCollection(), "configure_fonts" );
    new KAction( i18n( "Increase Font Sizes" ), "viewmag+", KShortcut(), this, SLOT( slotIncFontSizes() ), actionCollection(), "incFontSizes" );
    new KAction( i18n( "Decrease Font Sizes" ), "viewmag-", KShortcut(), this, SLOT( slotDecFontSizes() ), actionCollection(), "decFontSizes" );
}

void MainWindow::slotConfigureKeys()
{
  KKeyDialog::configure( actionCollection(), this );
}

void MainWindow::print()
{
    mDoc->view()->print();
}

void MainWindow::slotStarted(KIO::Job *job)
{
    if (job)
       connect(job, SIGNAL(infoMessage( KIO::Job *, const QString &)),
               SLOT(slotInfoMessage(KIO::Job *, const QString &)));

    History::self().updateActions();
}

void MainWindow::goInternalUrl( const KURL &url )
{
  slotOpenURLRequest( url, KParts::URLArgs() );
}

void MainWindow::slotOpenURLRequest( const KURL &url,
                                     const KParts::URLArgs &args )
{
    kdDebug( 1400 ) << "MainWindow::slotOpenURLRequest(): " << url.url() << endl;

    mNavigator->selectItem( url );

    QString proto = url.protocol().lower();

    if ( proto == "khelpcenter" ) return;

    bool own = false;

    if ( proto == "help" || proto == "glossentry" || proto == "about" ||
         proto == "man" || proto == "info" || proto == "cgi" ||
         proto == "ghelp" )
        own = true;
    else if ( url.isLocalFile() ) {
        KMimeMagicResult *res = KMimeMagic::self()->findFileType( url.path() );
        if ( res->isValid() && res->accuracy() > 40
             && res->mimeType() == "text/html" )
            own = true;
    }

    if ( !own ) {
        new KRun( url );
        return;
    }

    stop();

    mDoc->browserExtension()->setURLArgs( args );

    if ( proto == QString::fromLatin1("glossentry") ) {
        QString decodedEntryId = KURL::decode_string( url.encodedPathAndQuery() );
        slotGlossSelected( mNavigator->glossEntry( decodedEntryId ) );
        mNavigator->slotSelectGlossEntry( decodedEntryId );
    } else {
        History::self().createEntry();
        mDoc->openURL( url );
    }
}

void MainWindow::documentCompleted()
{
    kdDebug() << "MainWindow::documentCompleted" << endl;

    History::self().updateCurrentEntry( mDoc );
    History::self().updateActions();
}

void MainWindow::slotInfoMessage(KIO::Job *, const QString &m)
{
    statusBarMessage(m);
}

void MainWindow::statusBarMessage(const QString &m)
{
    statusBar()->changeItem(m, 0);
}

void MainWindow::slotOpenURL(const QString &url)
{
    if ( url.isEmpty() ) {
      slotShowHome();
    } else {
      openURL( KURL( url ) );
      mNavigator->selectItem( url );
    }
}

void MainWindow::openURL(const QString &url)
{
    slotOpenURL( url );
}

void MainWindow::openURL(const KURL &url)
{
    stop();
    KParts::URLArgs args;
    slotOpenURLRequest(url, args);
}

void MainWindow::slotGlossSelected(const GlossaryEntry &entry)
{
    stop();
    History::self().createEntry();
    mDoc->begin( "help:/khelpcenter/glossary" );
    mDoc->write( Glossary::entryToHtml( entry ) );
    mDoc->end();
}

void MainWindow::stop()
{
    mDoc->closeURL();
    History::self().updateCurrentEntry( mDoc );
}

void MainWindow::showHome()
{
    slotShowHome();
}

void MainWindow::slotShowHome()
{
    openURL( mNavigator->homeURL() );
    mNavigator->clearSelection();
}

void MainWindow::lastSearch()
{
    slotLastSearch();
}

void MainWindow::slotLastSearch()
{
    mDoc->lastSearch();
}

void MainWindow::enableLastSearchAction()
{
    mLastSearchAction->setEnabled( true );
}

void MainWindow::showSearchStderr()
{
  QString log = mNavigator->searchEngine()->errorLog();

  if ( !mLogDialog ) {
    mLogDialog = new LogDialog( this );
  }

  mLogDialog->setLog( log );
  mLogDialog->show();
  mLogDialog->raise();
}

void MainWindow::slotIncFontSizes()
{
  mDoc->slotIncFontSizes();
  updateZoomActions();
}

void MainWindow::slotDecFontSizes()
{
  mDoc->slotDecFontSizes();
  updateZoomActions();
}

void MainWindow::updateZoomActions()
{
  actionCollection()->action( "incFontSizes" )->setEnabled( mDoc->zoomFactor() + mDoc->zoomStepping() <= 300 );
  actionCollection()->action( "decFontSizes" )->setEnabled( mDoc->zoomFactor() - mDoc->zoomStepping() >= 20 );

  KConfig *cfg = kapp->config();
  {
    KConfigGroupSaver groupSaver( cfg, "General" );
    cfg->writeEntry( "Font zoom factor", mDoc->zoomFactor() );
    cfg->sync();
  }
}

void MainWindow::slotConfigureFonts()
{
  FontDialog dlg( this );
  if ( dlg.exec() == QDialog::Accepted )
    mDoc->slotReload();
}

// vim:ts=2:sw=2:et
