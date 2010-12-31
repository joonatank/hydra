/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *	Replaced spaces with tabs
 *	Fixed destructor segfaulting because deleting _treeItem multiple times
 *	Removed old commented out code
 */

/// Include interface header
#include "envcontrol.h"
/// Include GUI header
#include "ui_envcontrol.h"

/// Include vl headers
#include "base/filesystem.hpp"

/// Include QT headers
#include <QCheckBox>
#include <QRadioButton>



const QStringList PLUGIN_FILES = ( QStringList() << "*.so");
const QStringList EQC_FILES = ( QStringList() << "*.eqc");
const QStringList TRACKING_FILES = ( QStringList() << "*.trc");



EnvTreeItem::EnvTreeItem( QTreeWidgetItem *parent )
    : QTreeWidgetItem( parent )
{
	setText(0, "Environment");

	plugins = new QTreeWidgetItem(this);
	plugins->setText(0, "Plugins");

	eqc = new QTreeWidgetItem(this);
	eqc->setText(0, "Equalizer");

	tracking = new QTreeWidgetItem(this);
	tracking->setText(0, "Tracking");
}

EnvControl::EnvControl( QTreeWidgetItem *treeParent, QWidget *stackBase, QVBoxLayout* a) :
	ui( new Ui::EnvControl() ),
	_envSet( ),
	_treeItem( new EnvTreeItem( treeParent ) )
{
	ui->setupUi(this);
	this->setParent( stackBase );
	a->addWidget(this);

	_pageMap.insert( _treeItem, ui->pageEnv );
	_pageMap.insert( _treeItem->plugins, ui->pagePlugins );
	_pageMap.insert( _treeItem->eqc, ui->pageEqc );
	_pageMap.insert( _treeItem->tracking, ui->pageTracking );

	connect( treeParent->treeWidget(), SIGNAL(itemActivated(QTreeWidgetItem*,int)),
			 this, SLOT(changePage(QTreeWidgetItem*)));
}

EnvControl::~EnvControl( void )
{
	delete ui;
}

void EnvControl::changePage( QTreeWidgetItem *item )
{
	if( _pageMap.contains(item) )
	{
		ui->stack->setCurrentWidget( _pageMap.value(item) );
		this->show();
	}
	else
	{ this->hide(); }
}


bool EnvControl::openEnv( QString const &path )
{
	ui->pathEnv->setText(path);
	_file = path;

	_envSet.reset( new vl::EnvSettings );
	vl::EnvSettingsSerializer envSer(_envSet);
	//reading file with serializer to settings
	if( envSer.readString( vl::readFileToString( path.toStdString() ) ) )
	{
		updateEnv();
		return true;
	}
	else
	{ return false; }
}


void EnvControl::updateEnv( void )
{
	// Setting dir to environment file
	QFileInfo file(ui->pathEnv->text());
	QDir dir(file.path());

	dir.cd("plugins");
	updatePluginsTable(dir);

	dir.cdUp();
	dir.cd("tracking");
	updateTrackingTable(dir);

	dir.cdUp();
	dir.cd("eqc");
	updateEqcTable(dir);
}

void EnvControl::updatePluginsTable( QDir dir )
{
	dir.setNameFilters(PLUGIN_FILES);
	QFileInfoList listFromFolder(dir.entryInfoList(QDir::Files, QDir::Name));

	for( unsigned int i = 0; i < _envSet->getPlugins().size(); i++ )
	{

		QCheckBox* but = new QCheckBox(ui->tablePlugins);

		QString name = QString::fromStdString(_envSet->getPlugins().at(i).first);
		but->setText(name);

		if( _envSet->getPlugins().at(i).second == false )
		{ but->setCheckState(Qt::Unchecked); }
		else
		{ but->setCheckState(Qt::Checked); }

		// Checking through plugins file to see that all defined plugins are found
		// If plugin is missing it will be disabled
		bool found = false;
		for( int u = 0; u < listFromFolder.size(); u++ )
		{
			if(  name == listFromFolder.at(u).baseName() )
			{
				found = true;
			}
		}
		if(!found)
		{ but->setDisabled(true); }

		ui->tablePluginsLayout->addWidget(but);
    }
}



void EnvControl::updateTrackingTable( QDir dir )
{
	// listFromFolder consist of all files from the [en file path]/[folder]
	dir.setNameFilters(TRACKING_FILES);
	QFileInfoList listFromFolder(dir.entryInfoList(QDir::Files, QDir::Name));

	// string list of all active items from the envSet
	QList<QString> listOfActive;
	for( unsigned int i = 0; i < _envSet->getTracking().size(); i++ )
	{
		listOfActive << QString::fromStdString( _envSet->getTracking().at(i).file );
	}

	// Making list from all items from the folder
	for( int u = 0; u < listFromFolder.size(); u++ )
	{
		QCheckBox* but = new QCheckBox(ui->tableTracking);
		QString baseName = listFromFolder.at(u).baseName();
		QString fileName = listFromFolder.at(u).fileName();

		but->setText( baseName );

		if( listOfActive.indexOf(fileName) < 0 )
		{
			but->setCheckState(Qt::Unchecked);
		}
		else
		{
			but->setCheckState(Qt::Checked);
			//removing found item -> possibility to check if there is some plugins not in the folder
			listOfActive.erase(listOfActive.begin()+listOfActive.indexOf(fileName));
		}
		ui->tableTrackingLayout->addWidget(but);
	}

	//Adding to list not found items greyed
	for( int l = 0; l < listOfActive.size(); l++ )
	{
		QCheckBox* but = new QCheckBox(ui->tableTracking);
		QString name = QFileInfo(listOfActive.at(l)).baseName();
		but->setText( name );
		but->setCheckState(Qt::Checked);
		but->setDisabled(true);

		ui->tableTrackingLayout->addWidget(but);
	}
}



void EnvControl::updateEqcTable( QDir dir )
{
	// listFromFolder consist of all files from the [en file path]/[folder]
	dir.setNameFilters(EQC_FILES);
	QFileInfoList listFromFolder(dir.entryInfoList(QDir::Files, QDir::Name));

	// Active one
	QString active = QString::fromStdString(_envSet->getEqc());
	bool found = false;

	// Making list from all items from the folder
	for( int u = 0; u < listFromFolder.size(); u++ )
	{
		QRadioButton* but = new QRadioButton(ui->tableEqc);
		QString baseName = listFromFolder.at(u).baseName();
		QString fileName = listFromFolder.at(u).fileName();

		but->setText( baseName );

		if( fileName == active )
		{
			but->setDown(true);
			found = true;
		}
		ui->tableEqcLayout->addWidget(but);
	}

	//Adding to list not found items greyed
	if( !found )
	{
		QRadioButton* but = new QRadioButton(ui->tableEqc);
		QString name = QFileInfo(active).baseName();
		but->setText( name );
		but->setDown(true);
		but->setDisabled(true);

		ui->tableEqcLayout->addWidget(but);
	}
}

QString const &EnvControl::getFile( void ) const
{
	return _file;
}

QString const &EnvControl::getCommand( void ) const
{
	return ui->commandLine->text();
}

void EnvControl::writeSettings( QSettings *settings )
{
	settings->setValue("file", _file);
	settings->setValue("command", ui->commandLine->text());
	settings->setValue("expanded", _treeItem->isExpanded());
	settings->setValue("hidden", this->isHidden());
	settings->setValue("currPage", ui->stack->currentIndex());
	settings->beginGroup("treeItem");
	settings->setValue("env", _treeItem->isSelected());
	settings->setValue("plugins", _treeItem->plugins->isSelected());
	settings->setValue("eqc", _treeItem->eqc->isSelected());
	settings->setValue("tracking", _treeItem->tracking->isSelected());
	settings->endGroup();
}

void EnvControl::readSettings( QSettings *settings )
{
	ui->commandLine->setText(settings->value("command", "").toString());
	_treeItem->setExpanded(settings->value("Expanded", false).toBool());
	if(settings->value("hidden", true).toBool())
	{
		this->hide();
	}
	ui->stack->setCurrentIndex(settings->value("currPage", 0).toInt());
	settings->beginGroup("treeItem");
	_treeItem->setSelected(settings->value("env", false).toBool());
	_treeItem->plugins->setSelected(settings->value("plugins", false).toBool());
	_treeItem->eqc->setSelected(settings->value("eqc", false).toBool());
	_treeItem->tracking->setSelected(settings->value("tracking", false).toBool());
	settings->endGroup();
}
