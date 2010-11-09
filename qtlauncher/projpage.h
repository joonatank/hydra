#ifndef PROJ_PAGE_H
#define PROJ_PAGE_H

#include "base/projsettings.hpp"

#include "ui_projpage.h"

#include <QTreeWidgetItem>
#include <QDir>
#include <QSettings>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QCheckBox>



namespace Ui {
    class ProjPage;
}



class ProjPage : public QWidget
{
	Q_OBJECT

public :
	ProjPage( vl::ProjSettingsRefPtr projSet,
			  QWidget* base, QTreeWidgetItem* treeParent,
			  QString const & name );

	~ProjPage( void );

	void addScene(  QFileInfo const &file,
					QString const &name = "",
					bool set = true,
					QString const &attachto_scene = "",
					QString const &attachto_point = "" );

	void addMap( QFileInfo const &file, bool use = true );
	void addScript( QFileInfo const &file, bool use = true );


	void setName( QString const &name );

	QString getName() { return ui->name->text(); }

	QTreeWidgetItem* getTreeItem() { return _treeItem; }

	void writeSettings(QSettings* settings);
	void readSettings(QSettings* settings);

	void update();


public slots :
	void changePage(QTreeWidgetItem* item);

signals:
	void launch_clicked(QString const &);

private:
	Ui::ProjPage *ui;

	vl::ProjSettingsRefPtr _projSet;

	QTreeWidgetItem* _treeItem;
	bool existsIn(QString const &folder, QString const &item);
	QFileInfoList getDir(QString const & folderName);
	QList<QCheckBox*> _scenes;

	vl::ProjSettings::Case* _case;

	void updateMapsCombo();

private slots:
	void on_scriptsScrollArea_customContextMenuRequested(QPoint pos);
	void on_mapsCombo_activated(QString map);
	void on_mapScrollWidget_customContextMenuRequested(QPoint pos);
	void on_launch_clicked();
	void map_toggled(QWidget* but);
	void script_toggled(QWidget* but);

	void rmMap(QWidget* but);
	void rmScript(QWidget* but);

};



#endif
