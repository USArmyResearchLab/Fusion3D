#pragma once

#include <QDialog>
#include "ui_dialogCadOpen.h"

class vector_index_class;

class dialogCadOpen : public QDialog, public Ui::dialogCadOpen
{
	Q_OBJECT

	vector_index_class *vector_index;
	QStringList fileNames;

private slots:
	void doBrowse();
	void doLoad();

public:
	dialogCadOpen(QWidget *parent = Q_NULLPTR);
	~dialogCadOpen();

	int register_vector_index_class(vector_index_class *vector_index_in);
};
