/**********************************************************************
 *<
	FILE:  notetrck.h

	DESCRIPTION:  Note track plug-in class

	CREATED BY: Rolf Berteig

	HISTORY: created July 20, 1995

 *>	Copyright (c) 1994, All Rights Reserved.
 **********************************************************************/


#ifndef __NOTETRCK_H__
#define __NOTETRCK_H__

class NoteTrack : public ReferenceTarget {
	public:
		SClass_ID SuperClassID() {return SClass_ID(NOTETRACK_CLASS_ID);}
		RefResult AutoDelete() {return REF_SUCCEED;}
	};

class NoteAnimProperty : public AnimProperty {
	public:		
		NoteTrack *note;
		DWORD ID() {return PROPID_NOTETRACK;}

		NoteAnimProperty(NoteTrack *n=NULL) {note = n;}
		~NoteAnimProperty() {if (note) note->DeleteMe();}
	};


CoreExport NoteTrack *NewDefaultNoteTrack();


#endif // __NOTETRCK_H__

