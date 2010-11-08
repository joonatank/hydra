#ifndef ENV_CONTROL_H
#define ENV_CONTROL_H


/// Include QT headers
#include <QTableWidgetItem>
#include <QTreeWidgetItem>
#include <QListWidget>
#include <QDir>
#include <QStringList>
#include <QSettings>
#include <QVBoxLayout>

/// Include vl headers
#include "base/envsettings.hpp"

namespace Ui {
    class EnvControl;
}




class EnvTreeItem : public QTreeWidgetItem
{
public :
    EnvTreeItem( QTreeWidgetItem *parent );

    virtual ~EnvTreeItem( void ) {}
    QTreeWidgetItem *plugins;
    QTreeWidgetItem *eqc;
    QTreeWidgetItem *tracking;

private :


};




class EnvControl : public QWidget
{
	Q_OBJECT

public :
	EnvControl( QTreeWidgetItem *treeParent, QWidget *stackBase, QVBoxLayout* a);
	virtual ~EnvControl( void );

	bool openEnv(QString const &path);
	void updateEnv();

	void updatePluginsTable(QDir dir);
	void updateTrackingTable(QDir dir);
	void updateEqcTable(QDir dir);

	QString getFile();
	QString getCommand();

	void readSettings(QSettings* settings);
	void writeSettings(QSettings* settings);

public slots :
	void changePage(QTreeWidgetItem* item);

private :
	Ui::EnvControl *ui;
	vl::EnvSettingsRefPtr _envSet;
	EnvTreeItem* _treeItem;
	QString _file;


	QMap<QTreeWidgetItem*, QWidget*> _pageMap;

};


#endif
