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

#pragma once

#include <QString>
#include <QAbstractItemModel>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>


/**
 * @brief The TreeNode struct
 *
 * Used as the internalpointer member of QModelIndex objects.
 * This structure is used to map the internal model to Qt's QAbstractItemModel
 * paradigm.
 *
 */
struct TreeNode {

  TreeNode          *parent;    /**< Address of the parent TreeNode (nullptr for root) */
  QList<TreeNode *>  children;  /**< List of chil elements under this node */
  QString            key;       /**< The key by which this node is known (displayed in column 0).
                                  This string is empty ("") for array elements. */
  QJsonValue         data;      /**< The value of the node. This is displayed in the tree view. */


  TreeNode(TreeNode *p, const QString &k, const QJsonValue &d) : parent(p), key(k), data(d) {}
  ~TreeNode() {}

  /**
   * @brief row
   *
   * @return Returns an integer corresponding to the row of this node under its
   * parent. For the root of the tree, the row count is always 0.
   */
  int row() const {

    if(parent != nullptr)
      return parent->children.indexOf(const_cast<TreeNode *>(this));

    return 0;
  }
};


/**
 * @brief The JsonTreeModel class
 *
 * An implementation of the QAbstractItemModel class. This class re-implements
 * the necessary virtual functions to display and edit model elements in a QTreeView.
 *
 */
class JsonTreeModel : public QAbstractItemModel
{
  Q_OBJECT

private:
  bool           modified;
  TreeNode      *root;

  std::string indent(int level);
  void freeTraverse(TreeNode *node);
  void traverse(const QJsonObject &jobj, TreeNode *parent, int level);
  void traverse(const QJsonArray &jarr, TreeNode *parent, int level);
  void updateNode(TreeNode *node, int i, const QString oldKey, const QString newKey, const QJsonValue value);
  QJsonValue jsonFromVariant(const QVariant &var);

public:
  explicit JsonTreeModel(QObject *parent = 0);
  JsonTreeModel(const QString &file, QJsonParseError *err, QObject *parent = 0);
  virtual ~JsonTreeModel();

  bool isModified();
  void resetModified();

  QJsonDocument toJsonDocument();

  // Header:
  QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

  // Basic functionality:
  QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
  QModelIndex parent(const QModelIndex &index) const override;

  int rowCount(const QModelIndex &parent = QModelIndex()) const override;
  int columnCount(const QModelIndex &parent = QModelIndex()) const override;

  QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
  bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

  Qt::ItemFlags flags(const QModelIndex& index) const override;

};
