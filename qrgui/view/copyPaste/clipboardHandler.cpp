﻿#include <QtWidgets/QApplication>
#include <QtGui/QClipboard>

#include "clipboardHandler.h"
#include "pasteGroupCommand.h"
#include "../editorViewScene.h"

using namespace qReal;

ClipboardHandler::ClipboardHandler(EditorViewScene *scene)
	: mScene(scene), mMVIface(NULL)
{
}

void ClipboardHandler::setMVIface(EditorViewMViface *mvIface)
{
	mMVIface = mvIface;
}

void ClipboardHandler::setController(Controller * const controller)
{
	mController = controller;
}

void ClipboardHandler::copy()
{
	QList<NodeElement *> nodes = getNodesForCopying();
	QList<NodeData> nodesData = getNodesData(nodes);

	QList<EdgeElement *> edges = getEdgesForCopying(nodes);
	QList<EdgeData> edgesData = getEdgesData(edges);

	pushDataToClipboard(nodesData, edgesData);
}

QList<NodeData> ClipboardHandler::getNodesData(QList<NodeElement *> const &nodes) const
{
	QList<NodeData> nodesData;
	foreach (NodeElement* node, nodes) {
		nodesData << node->data();
	}
	return nodesData;
}

QList<EdgeData> ClipboardHandler::getEdgesData(QList<EdgeElement *> const &edges) const
{
	QList<EdgeData> edgesData;
	foreach (EdgeElement* edge, edges) {
		edgesData << edge->data();
	}
	return edgesData;
}

QList<NodeElement *> ClipboardHandler::getNodesForCopying() const
{
	QList<NodeElement *> nodes;
	foreach (QGraphicsItem *item, mScene->selectedItems()) {
		NodeElement *node = dynamic_cast<NodeElement *>(item);
		if (node && !mScene->selectedItems().contains(node->parentItem())) {
			nodes << node;
		}
	}
	foreach (NodeElement *node, nodes) {
		addChildren(node, nodes);
	}

	return nodes;
}

QList<EdgeElement *> ClipboardHandler::getEdgesForCopying(QList<NodeElement *> const &nodes) const
{
	QSet<EdgeElement *> edgeSet;

	foreach (QGraphicsItem *item, mScene->selectedItems()) {
		EdgeElement *edge = dynamic_cast<EdgeElement *>(item);
		if (edge) {
			edgeSet << edge;
		}
	}

	// add children. because edges.parent is root diagram
	foreach (NodeElement *node, nodes) {
		if (node->childItems().size() > 0) {
			foreach (QGraphicsItem *item, mScene->collidingItems(node)) {
				EdgeElement *edge = dynamic_cast<EdgeElement *>(item);
				if (edge) {
					edgeSet << edge;
				}
			}
		}
	}

	return edgeSet.toList();
}

void ClipboardHandler::addChildren(NodeElement *node, QList<NodeElement *> &nodes) const
{
	foreach (QGraphicsItem *item, node->childItems()) {
		NodeElement *child = dynamic_cast<NodeElement *>(item);
		if (child && !nodes.contains(child)) {
			nodes << child;
			addChildren(child, nodes);
		}
	}
}

void ClipboardHandler::pushDataToClipboard(QList<NodeData> const &nodesData, QList<EdgeData> const &edgesData) const
{
	QByteArray data;
	QDataStream stream(&data, QIODevice::WriteOnly);

	stream << nodesData;
	stream << edgesData;

	QMimeData *mimeData = new QMimeData();
	mimeData->setData("application/x-real-uml-model-data", data);

	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setMimeData(mimeData);
}

void ClipboardHandler::paste(bool isGraphicalCopy)
{
	commands::PasteGroupCommand *pasteCommand = new commands::PasteGroupCommand(mScene, mMVIface, isGraphicalCopy);
	if (!pasteCommand->isEmpty()) {
		mController->execute(pasteCommand);
	}
}
