/*
The MIT License (MIT)

Copyright (c) 2016 Charles Bushakra

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include <QFileDialog>
#include <QMessageBox>
#include "mainwindow.h"
#include "ui_mainwindow.h"


/**
 * @brief MainWindow::MainWindow
 * @param parent : Parent QObject for the main window.
 *
 * Initialize the UI.
 * Model pointer is initially nullptr.
 */

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
  ui->setupUi(this);
  ptm = nullptr;
}

/**
 * @brief MainWindow::~MainWindow
 *
 * Delete the ui object.
 * The model object is destroyed automatically by Qt because the MainWindow
 * is its parent.
 *
 */
MainWindow::~MainWindow()
{
  delete ui;
}


/**
 * @brief MainWindow::querySave
 *
 * If the model has been modified, ask to save or discard the changes.
 * A cancel button is included to give the user the opportunity to cancel
 * the current operation (open/quit) without saving.
 *
 * @return Returns -1 if the current operation (open/quit) should be cancelled; 0 otherwise.
 */

int MainWindow::querySave()
{
  int res;

  res = 0;
  if(ptm != nullptr) {
      if(ptm->isModified() == true) {

          int choice;
          QMessageBox::StandardButtons buttons;

          buttons = QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel;
          choice = QMessageBox::question(this, "Save file?", "Save the modified file?", buttons, QMessageBox::Save);

          switch(choice) {
            case QMessageBox::Save:
              saveFile();
              res = 0;
              break;
            case QMessageBox::Discard:
              res = 0;
              break;
            case QMessageBox::Cancel:
              res = -1;
              break;
          }
      }
  }

  return res;
}

/**
 * @brief MainWindow::openFile
 *
 * Slot connected to the triggered signal of actionOpenFile.
 * Presents a system File Open dialog to the user.
 */

void MainWindow::openFile()
{
  QString         jsonFile;
  QJsonParseError err;
  JsonTreeModel  *tempModel;


  if(querySave() >= 0) {

      jsonFile = QFileDialog::getOpenFileName(this, QString("Open JSON File"), QString("/"));
      if(jsonFile.isNull() == false) {

          tempModel = new JsonTreeModel(jsonFile, &err, this);
          if(err.error == QJsonParseError::NoError) {

              // Free the old model
              if(ptm != nullptr) {
                  ui->treeView->setModel(nullptr);
                  delete ptm;
                }

              // Set the new model
              ptm = tempModel;
              ui->treeView->setModel(ptm);
              ui->currentFile->setText(jsonFile);

            }
          else {
              ui->statusBar->showMessage(err.errorString() + " at position " + QString::number(err.offset));
            }
      }
  }
}


/**
 * @brief MainWindow::saveFile
 *
 * Slot connected to the triggered signal of actionSaveFile.
 * Writes the current PTreeModel back to disc.
 * The file is saved under the same file name that was opened.
 *
 */
void MainWindow::saveFile()
{

  if(ptm != nullptr) {
      QJsonDocument doc = ptm->toJsonDocument();
      QByteArray jsonBytes = doc.toJson();

      QFile outFile(ui->currentFile->text());
      outFile.open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate);
      outFile.write(jsonBytes);
      outFile.close();

      // Reset the model's modified flag, since changes have been saved.
      ptm->resetModified();
      ui->statusBar->showMessage("Saved.");
  }
  else {
      ui->statusBar->showMessage("No file to save.");
  }

}


/**
 * @brief MainWindow::appQuit
 *
 * Slot connected to the triggered signal of actionQuit.
 * Closes the application.
 * @note the JsonTreeModel is deleted by Qt since the MainWindow object is its
 * parent.
 *
 *
 */
void MainWindow::appQuit()
{
  if(querySave() >= 0)
    qApp->quit();
}
