# qjtree
A small project to demonstrate how to implement a Qt QAbstractItemModel class.

This project implements a concrete class, JsonTreeModel, which can be used with
a Qt QTreeView to display and edit an existing JSON file.

The JsonTreeModel currently allows the user to edit existing key/values and
array items in the file. It currently does not allow addition of new items or deletions.

To initiate editing, double-click the mouse on either the key, or the value.

To cancel the edit, presse the ESC key.

For JSON arrays, items are shown with a "key" of [n], where n is the index of the item.
The program does not allow editing of array indices shown in the key column in the QTreeView.

The JsonTreeModel class performs rudimentary type checking in the following way:

- If a value contains a decimal point ('.') an attempt will be made to convert it to a double.
- If there is no decimal point, an attempt will be made to convert it to an integer.
- If either of the above conversions fail, the value is converted to a string.

# Qt Classes Used

The following Qt classes are demonstrated in this project:

- QAbstractItemModel
- QJsonDocument
- QJsonObject
- QJsonArray
- QJsonValue
- QVariant
- QTreeView


