//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_COPY_OPERATIONS_H
#define RME_COPY_OPERATIONS_H

#include "map/position.h"

class Editor;
class CopyBuffer;

class CopyOperations {
public:
	static void copy(Editor& editor, CopyBuffer& buffer, int floor);
	static void cut(Editor& editor, CopyBuffer& buffer, int floor);
	static void paste(Editor& editor, CopyBuffer& buffer, const Position& toPosition);
};

#endif
