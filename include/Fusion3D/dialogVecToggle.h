#pragma once

#include <QDialog>
#include "ui_dialogVecToggle.h"

class vector_index_class;
class QCheckBox;

/**
Qt dialog for toggling on and off individual vector overlay files.
\n
Qt dialog for toggling on and off individual vector overlay files.
The entire set of vector overlays can be toggled on and off using the 'Vect' button at the left of the main window.
*/
class dialogVecToggle : public QDialog, public Ui::dialogVecToggle
{
	Q_OBJECT

	QCheckBox **fileBox;

	int nVectorCheckBox;
	int* visVal;
	vector_index_class *vector_index;
	int populateMenu();

private slots:
	void doApply();
	void doToggle(bool val);


public:
	dialogVecToggle(QWidget *parent = Q_NULLPTR);
	~dialogVecToggle();

	int register_vector_index_class(vector_index_class *vector_index_in);

};
