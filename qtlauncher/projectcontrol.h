/**	Joonas Reunamo <joonas.reunamo@tut.fi>
 *	2010-10
 *
 *	Updated by Joonatan Kuosa <joonatan.kuosa@tut.fi>
 *	2010-12
 *	Replaced spaces with tabs
 */

#ifndef PROJ_CONTROL_H
#define PROJ_CONTROL_H

#include "base/projsettings.hpp"
#include "projpage.h"

#include <QTreeWidgetItem>
#include <QDir>
#include <QSettings>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QCheckBox>

namespace Ui {
	class ProjControl;
}

class ProjControl : public QWidget
{
	Q_OBJECT

public :
	ProjControl( QTreeWidgetItem *treeParent, QWidget *base );

	~ProjControl( void );

	bool openProj( QString const &fileStr );

	QTreeWidgetItem *getProjTreeItem( void ) { return _treeItem; }

	void writeSettings( QSettings *settings );
	void readSettings( QSettings *settings );

	void writeProj(QString &fileStr);

	bool saveToFile(QString const &fileStr = "");

	QString const &getFile( void ) const { return _file; }

public slots :

signals:
	void launched(QString const &fileName, QString const &caseName = "" );

private:
	vl::ProjSettingsRefPtr _projSet;
	QTreeWidgetItem* _treeItem;
	QString _file;
	QDir _path; //path to find project action maps and project resources
	QList<ProjPage*> _pages;
	ProjPage* _currPage;
	QTreeWidgetItem* _currTreeItem;
	QTreeWidgetItem* _treeParent;
	QWidget* _base;

	void createProj( void );
	void addScenesAndMaps( QString const &pageName, ProjPage* page );
	void addCase( QString const &name );

private slots:
	void launch_clicked(QString const &caseName);

};	// class ProjControl


#endif	// PROJ_CONTROL_H
