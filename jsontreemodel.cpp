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

#include <string>
#include <iostream>
#include <stdexcept>
#include <QFile>
#include <QByteArray>
#include "jsontreemodel.h"


/**
 * @brief JsonTreeModel::JsonTreeModel
 * @param parent: QObject parent for this object
 *
 * Initialize the root document to nullptr.
 *
 */

JsonTreeModel::JsonTreeModel(QObject *parent) : QAbstractItemModel(parent)
{
  root = nullptr;
}


/**
 * @brief JsonTreeModel::JsonTreeModel
 * @param file: Name of the JSON file to open
 * @param err: Address of a QJsonParseError object
 * @param parent: QObject parent for this object
 *
 * Open and parse the given JSON file.
 * Caller must check the err structure for a parse error.
 * If err->error == QJsonParseError::NoError then the file was loaded successfully.
 * Otherwise the err structure will contain Qt provided error information.
 *
 */

JsonTreeModel::JsonTreeModel(const QString &file, QJsonParseError *err, QObject *parent) : JsonTreeModel(parent)
{
  QFile jsonFile(file);
  QJsonDocument rootDoc;
  QByteArray jsonContents;



  jsonFile.open(QIODevice::ReadOnly | QIODevice::Text);
  jsonContents = jsonFile.readAll();
  rootDoc = QJsonDocument::fromJson(jsonContents, err);
  jsonFile.close();

  if(err->error == QJsonParseError::NoError) {
      if(rootDoc.isObject()) {
          root = new TreeNode(nullptr, QString("root"), rootDoc.object());
          traverse(rootDoc.object(), root, 0);
      }
      else {
          root = new TreeNode(nullptr, QString("root"), rootDoc.array());
          if(rootDoc.isArray())
            traverse(rootDoc.array(), root, 0);
      }
  }

}

/**
 * @brief JsonTreeModel::~JsonTreeModel
 *
 * If a root document was allocated, it is deleted.
 *
 */

JsonTreeModel::~JsonTreeModel()
{
  if(root != nullptr)
    freeTraverse(root);
}


bool JsonTreeModel::isModified()
{
  return modified;
}


void JsonTreeModel::resetModified()
{
  modified = false;
}


/**
 * @brief JsonTreeModel::indent
 *
 * Utility function that creates a string to use for padding.
 * Used when printing out the tree, the pad string can indent the node value
 * according to its depth in the tree.
 *
 * @param level: Current level of a node in the tree.
 *
 * @return Returns a string with 2 spaces for each level.
 */

std::string JsonTreeModel::indent(int level)
{
  std::string s;
  for(int i=0; i<level; i++)
    s += "  ";

  return s;
}


/**
 * @brief JsonTreeModel::toJsonDocument
 * @return Returns a QJsonDocument that is constructed from the current
 * tree structure.
 */

QJsonDocument JsonTreeModel::toJsonDocument()
{
  if(root->data.type() == QJsonValue::Array)
    return QJsonDocument(root->data.toArray());
  else
    return QJsonDocument(root->data.toObject());
}


/**
 * @brief JsonTreeModel::freeTraverse
 *
 * Performs a traversal of the tree starting from the given node.
 * This function is used by the destructor to delete the document tree.
 *
 * @param node: Node at which to start the traversal.
 *
 */

void JsonTreeModel::freeTraverse(TreeNode *node)
{
  for(auto child : node->children) {
      freeTraverse(child);
  }

  delete node;
}


/**
 * @brief JsonTreeModel::traverse
 *
 * Perform a tree traversal from a node that is of type QJsonObject.
 *
 * This function converts the document from QJson* objects into
 * TreeNode structures.
 *
 * The traversal is recursive, for each key/value pair contained in the jobj
 * node.
 *
 * @param jobj: Current node; JsonObject
 * @param parent: The parent TreeNode for jobj
 * @param level: Depth of jobj in the tree. Used for debugging/printing node values.
 *
 * @see traverse(const QJsonArray &jarr, TreeNode *parent, int level)
 */

void JsonTreeModel::traverse(const QJsonObject &jobj, TreeNode *parent, int level)
{
  TreeNode *newNode;


  for(auto key : jobj.keys()) {

      QJsonValue val = jobj[key];
      switch(val.type()) {

        case QJsonValue::Type::Object:
         newNode = new TreeNode(parent, key, val);
         parent->children.append(newNode);
         traverse(val.toObject(), newNode, level+1);
         break;

       case QJsonValue::Type::Array:
         newNode = new TreeNode(parent, key, val);
         parent->children.append(newNode);
         traverse(val.toArray(), newNode, level+1);
         break;

       default:
         newNode = new TreeNode(parent, key, val);
         parent->children.append(newNode);
         break;
     }
  }
}


/**
 * @brief JsonTreeModel::traverse
 *
 * Perform a tree traversal from a node that is of type QJsonArray.
 *
 * With traverse(QJsonObject &,...) this function converts the document from
 * QJson* objects into TreeNode structures.
 *
 * The traversal is recursive, for each item in the QJsonArray.
 *
 * @param jarr: Current node; JsonArray
 * @param parent: The parent TreeNode for jarr
 * @param level: Depth of jarr in the tree. Used for debugging/printing node values.
 *
 * @see const QJsonObject &jobj, TreeNode *parent, int level)
 */

void JsonTreeModel::traverse(const QJsonArray &jarr, TreeNode *parent, int level)
{
  TreeNode *newNode;


  for(int i = 0; i < jarr.size(); i++) {

      QJsonValue val = jarr[i];
      switch(val.type()) {

        case QJsonValue::Type::Object:
          newNode = new TreeNode(parent, QString(""), val);
          parent->children.append(newNode);
          traverse(val.toObject(), newNode, level+1);
          break;

        case QJsonValue::Type::Array:
          newNode = new TreeNode(parent, QString(""), val);
          parent->children.append(newNode);
          traverse(val.toArray(), newNode, level+1);
          break;

        default:
          newNode = new TreeNode(parent, QString(""), val);
          parent->children.append(newNode);
          break;
      }
  }

}


/**
 * @brief JsonTreeModel::jsonFromVariant
 *
 * Create a QJsonValue object from a QVariant.
 *
 * This function performs some rudimamentary type checking as follows:
 * @li If the variant string contains a '.' character then we attempt to create a double.
 * @li If double conversion fails, try to convert to an integer.
 * @li If double/integer conversion fails, then create a string.
 *
 * @param var: The QVariant to convert
 * @return Returns a QJsonValue of type double/integer/string
 */

QJsonValue JsonTreeModel::jsonFromVariant(const QVariant &var)
{
  double dVal;
  int iVal;
  QString sVal;
  QJsonValue resultValue;


  sVal = var.toString();
  try {
    if(sVal.contains('.') == true) {
      dVal = std::stod(sVal.toStdString());
      resultValue = QJsonValue(dVal);
    }
    else {
      iVal = std::stoi(sVal.toStdString());
      resultValue = QJsonValue(iVal);
    }
  }
  catch(std::invalid_argument &ia) {
    resultValue = QJsonValue(sVal);
  }

  return resultValue;
}


/**
 * @brief JsonTreeModel::updateNode
 *
 * This function is called from setData(). It is the interface between the UI and the
 * internal TreeNode model.
 *
 * Because Qt does everything by value, we have to update the TreeNode as follows:
 * @li Update the values in the current TreeNode
 * @li Get a QJsonObject or QJsonArray object from the node's parent
 * @li For a QJsonObject, remove the old key/value pair and insert the new key/value pair.
 * @li For a QJsonArray, replace the value at index i, which is given by the row in the QModelIndex.
 * @li Replace the parent node's @c data item with the new one.
 * @li Traverse @b up the tree so that the changes are propagated to the root of the document.
 *
 * @param node: The node being updated - starts at the node updated in the UI
 * @param i: The index/row of the node being updated
 * @param oldKey: Old key of the node being updated
 * @param newKey: New key of the node being updated
 * @param value: New value for the node
 *
 */

void JsonTreeModel::updateNode(TreeNode *node, int i, const QString oldKey, const QString newKey, const QJsonValue value)
{
  QJsonObject jobj;
  QJsonArray jarr;


  if(node == nullptr)
    return;


  node->key = newKey;
  node->data = value;
  if(node->parent != nullptr) {
      switch(node->parent->data.type()) {

        case QJsonValue::Type::Object:
          jobj = node->parent->data.toObject();
          jobj.remove(oldKey);
          jobj.insert(newKey, value);
          node->parent->data = QJsonValue(jobj);
          break;

        case QJsonValue::Type::Array:
          jarr = node->parent->data.toArray();
          jarr[i] = value;
          node->parent->data = QJsonValue(jarr);
          break;

        default:
          std::cout << "updateNode: parent type default.\n";
          break;
      }

      updateNode(node->parent, node->parent->row(), node->parent->key, node->parent->key, node->parent->data);
  }

}


/**
 * @brief JsonTreeModel::headerData
 *
 * Provide text for the two headers in the Qt tree view.
 * @note the check for Qt::DisplayRole is required otherwise nothing is displayed.
 *
 * @param section: The column number
 * @param orientation: Qt allows for horizontal or verticle headings
 * @param role: Qt role.
 *
 * @return Returns a string (as a variant) to use for the column heading.
 */

QVariant JsonTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
  Q_UNUSED(orientation);

  // Check for DisplayRole is required for Qt to display the header at all.
  if (role != Qt::DisplayRole)
      return QVariant();

  if(section == 0)
    return QVariant(QString("Key"));
  else
    return QVariant(QString("Value"));
}


/**
 * @brief JsonTreeModel::index
 *
 * This function maps from the Qt abstract model row/column to the model's
 * concept of row/column.
 *
 * The @c internalPointer item of QModelIndex stores a pointer to a TreeNode structure.
 *
 * @param row: The row index of the item under its parent item.
 * @param column: The column index of the item
 * @param parent: The index of the item's parent in the tree view.
 * @return Returns a QModelIndex that locates the corresponding item in the UI;
 * Returns an empty index if the @c row and @c column are out of bounds.
 *
 */

QModelIndex JsonTreeModel::index(int row, int column, const QModelIndex &parent) const
{
  TreeNode *parentNode, *childNode;

  // If the index is out of bounds, return an empty index.
  if(hasIndex(row, column, parent) == false) {
      return QModelIndex();
  }

  // If there is no parent, then Qt is asking us for the index of the root item.
  if(parent.isValid() == false) {
    parentNode = root;
  }
  else {
    parentNode = static_cast<TreeNode *>(parent.internalPointer());
  }

  childNode = parentNode->children[row];
  return createIndex(row, column, childNode);
}


/**
 * @brief JsonTreeModel::parent
 *
 * Similar to index(), this function receives a QModelIndex and returns
 * an index for its parent.
 *
 * @param index: The index to find the parent of
 * @return Returns a QModelIndex for the parent; Returns an empty index if the
 * parent is the root item.
 */

QModelIndex JsonTreeModel::parent(const QModelIndex &index) const
{

  if(!index.isValid())
    return QModelIndex();

  TreeNode *childNode  = static_cast<TreeNode *>(index.internalPointer());
  TreeNode *parentNode = childNode->parent;

  if(parentNode == root)
    return QModelIndex();

  return createIndex(parentNode->row(), 0, parentNode);
}


/**
 * @brief JsonTreeModel::rowCount
 *
 * Returns the number of rows (children) under the parent index.
 *
 * @param parent: Index of the parent item
 * @return Returns the number of children under the given parent.
 */

int JsonTreeModel::rowCount(const QModelIndex &parent) const
{
  TreeNode *parentNode;


  if(parent.isValid() == false)
      parentNode = root;
  else
      parentNode = static_cast<TreeNode *>(parent.internalPointer());

  return parentNode->children.count();
}


/**
 * @brief JsonTreeModel::columnCount
 *
 * Returns the number of columns under the parent index.
 *
 * @param parent: Index of the parent item
 * @return Returns 2 columns, regardless of parent.
 */

int JsonTreeModel::columnCount(const QModelIndex &parent) const
{
  Q_UNUSED(parent)

  // Two columns: Key, and Value.
  return 2;
}


/**
 * @brief JsonTreeModel::data
 *
 * Returns a variant holding the data at the given model index.
 *
 * @param index: The index for which to return data
 * @param role: The Qt role; we return an empty variant for all but DisplayRole
 * @return Returns a variant with the data to display at the given model index.
 */

QVariant JsonTreeModel::data(const QModelIndex &index, int role) const
{

  if(!index.isValid())
    return QVariant();

  if(role != Qt::DisplayRole)
    return QVariant();

  // Use the index to retrieve the internal pointer
  TreeNode *item = static_cast<TreeNode *>(index.internalPointer());

  // Column 1 is the data item, column 0 is the key
  if(index.column() == 1)
    return item->data.toVariant();
  else {
      if(item->key.isEmpty()) {
        QString temp;

        temp = temp.sprintf("[%d]", index.row());
        return QVariant(temp);
      }
      else
        return QVariant(item->key);
  }
}


/**
 * @brief JsonTreeModel::setData
 *
 * Called by Qt when the user edits an item in the tree view. Either the
 * key (column 0) or the value (column 1) has been changed.
 *
 * The updateNode() function is called to set the new value and to propagate the
 * changes up to the root of the document tree.
 *
 * Emits the dataChanged signal once the new value is set.
 *
 * @param index: The model index of the item that was edited.
 * @param value: The new value provided by the user via the UI.
 * @param role: The Qt role (not used in this function)
 * @return Returns true if the change was made; false otherwise.
 */

bool JsonTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{

  if(data(index, role) != value) {

      // Need to get the parent (object or array) and set the key/value
      // To update only the key, need to first remove the key, then add new key/value.
      TreeNode *item = static_cast<TreeNode *>(index.internalPointer());
      if(index.column() == 0) {

          // An empty key means the tree item corresponds to an item in a JsonArray.
          // Edits are disallowed, since it's not really a key.
          if(value.toString().isEmpty() == true)
            return false;

          QString newKey = value.toString();
          updateNode(item, index.row(), item->key, newKey, item->data);
      }
      else {
          if(index.column() == 1) {
              QJsonValue newValue = jsonFromVariant(value);
              updateNode(item, index.row(), item->key, item->key, newValue);
          }
      }

      modified = true;
      emit dataChanged(index, index, QVector<int>() << role);
      return true;
  }

  return false;
}


/**
 * @brief JsonTreeModel::flags
 *
 * Returns flags for the item at the given model index.
 * An index corresponding to the key of an array item (column 0, empty key)
 * is enabled and selectable but cannot be edited.
 *
 * All other items are enabled, selectable, and editable.
 *
 * @param index: Index of the item for which flags are returned
 * @return  A Qt::ItemFlags value for the item at the given index
 *
 */
Qt::ItemFlags JsonTreeModel::flags(const QModelIndex &index) const
{
  if (!index.isValid())
    return Qt::NoItemFlags;

  TreeNode *item = static_cast<TreeNode *>(index.internalPointer());

  // Do not allow edits on keys of array items.
  if(item->key.isEmpty() && index.column() == 0)
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
  else
    return Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

