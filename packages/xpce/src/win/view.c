/*  $Id$

    Part of XPCE
    Designed and implemented by Anjo Anjewierden and Jan Wielemaker
    E-mail: jan@swi.psy.uva.nl

    Copyright (C) 1992 University of Amsterdam. All rights reserved.
*/

#include <h/kernel.h>
#include <h/graphics.h>
#include <h/text.h>

static status
initialiseView(View v, Name name, Size size, DisplayObj display, Editor editor)
{ Editor e;

  if ( isDefault(editor) )
  { Size s2;

    if ( isDefault(size) && (s2 = getResourceValueObject(v, NAME_size)) )
      size = newObject(ClassSize, s2->w, s2->h, 0);

    TRY(e = get(v, NAME_createEditor, size, 0));
  } else
    e = editor;

  initialiseWindow((PceWindow) v, name,
		   getSizeGraphical((Graphical) e), display);

  return send(v, NAME_editor, e, 0);
}


static status
unlinkView(View v)
{ Editor e = v->editor;

  unlinkWindow((PceWindow) v);
  assign(v, editor, NIL);
  freeObject(e);

  succeed;
}


static status
editorView(View v, Editor editor)
{ if ( notNil(v->editor) )
  { assign(v, editor, NIL);
    send(v->editor, NAME_destroy, 0);
  }
  assign(v, editor, editor);
  send(editor, NAME_set, ZERO, ZERO, 0);
  send(v, NAME_display, editor, 0);
  send(v, NAME_resizeMessage,
       newObject(ClassMessage, editor, NAME_Size, Arg(2), 0), 0);
  assign(v, keyboard_focus, editor);

  succeed;
}


static Editor
getCreateEditorView(View v, Size size)
{ Editor e;
  Int w, h;

  if ( notDefault(size) )
  { w = size->w;
    h = size->h;
  } else
    w = h = DEFAULT;

  if ( (e = newObject(ClassEditor, DEFAULT, w, h, 0)) )
  { FontObj f;

    if ( (f = getResourceValueObject(v, NAME_font)) )
      send(e, NAME_font, f, 0);

    answer(e);
  }

  fail;
}


static Point
getSelectionView(View v)
{ answer(getSelectionEditor(v->editor));
}


status
requestGeometryView(View v, Int x, Int y, Int w, Int h)
{ Editor e = v->editor;

  if ( notDefault(w) )
    w = mul(w, getExFont(e->font));
  if ( notDefault(h) )
    h = mul(h, getHeightFont(e->font));

  return requestGeometryWindow((PceWindow) v, x, y, w, h);
}


		/********************************
		*           OVERRULE		*
		********************************/

static status
clearView(View v)
{ return clearEditor(v->editor);
}


static status
selectionView(View v, Int from, Int to)
{ return selectionEditor(v->editor, from, to);
}


static StringObj
getSelectedView(View v)
{ return getSelectedEditor(v->editor);
}


static status
normaliseView(View v, Int from, Int to)
{ return normaliseEditor(v->editor, from, to);
}


static status
scrollToView(View v, Int index)
{ return scrollToEditor(v->editor, index);
}


static status
formatView(View v, CharArray fmt, int argc, Any *argv)
{ return formatEditor(v->editor, fmt, argc, argv);
}


		 /*******************************
		 *	 CLASS DECLARATION	*
		 *******************************/

/* Type declarations */

static const char *T_format[] =
        { "char_array", "any ..." };
static const char *T_fromAint_toAint[] =
        { "from=int", "to=int" };
static const char *T_initialise[] =
        { "label=[name]", "size=[size]", "display=[display]", "editor=[editor]" };
static const char *T_requestGeometry[] =
        { "x=[int]", "y=[int]", "width=[int]", "height=[int]" };

/* Instance Variables */

static const vardecl var_view[] =
{ IV(NAME_editor, "editor", IV_GET,
     NAME_delegate, "Editor displayed")
};

/* Send Methods */

static const senddecl send_view[] =
{ SM(NAME_editor, 1, "editor", editorView,
     DEFAULT, "Associate editor with view"),
  SM(NAME_initialise, 4, T_initialise, initialiseView,
     DEFAULT, "Create from label, size, display and editor"),
  SM(NAME_requestGeometry, 4, T_requestGeometry, requestGeometryView,
     DEFAULT, "Map size to character units"),
  SM(NAME_unlink, 0, NULL, unlinkView,
     DEFAULT, "Unlink the editor"),
  SM(NAME_clear, 0, NULL, clearView,
     NAME_delete, "Overrule window behaviour"),
  SM(NAME_format, 2, T_format, formatView,
     NAME_format, "Formatted insert (see `string->format')"),
  SM(NAME_normalise, 2, T_fromAint_toAint, normaliseView,
     NAME_scroll, "Overrule window behaviour"),
  SM(NAME_scrollTo, 1, "int", scrollToView,
     NAME_scroll, "Overrule window behaviour"),
  SM(NAME_selection, 2, T_fromAint_toAint, selectionView,
     NAME_selection, "Overrule window behaviour")
};

/* Get Methods */

static const getdecl get_view[] =
{ GM(NAME_createEditor, 1, "editor", "size=[size]", getCreateEditorView,
     NAME_create, "Create the editor of the view"),
  GM(NAME_selected, 0, "string", NULL, getSelectedView,
     NAME_selection, "New string with contents of selection"),
  GM(NAME_selection, 0, "point", NULL, getSelectionView,
     NAME_selection, "New point with start and end of selection")
};

/* Resources */

static const resourcedecl rc_view[] =
{ RC(NAME_pen, "int", "0",
     "Pen (done by <-editor)"),
  RC(NAME_size, "size", "size(80,20)",
     "Default size in `characters x lines'")
};

/* Class Declaration */

static Name view_termnames[] = { NAME_label, NAME_displaySize, NAME_display, NAME_editor };

ClassDecl(view_decls,
          var_view, send_view, get_view, rc_view,
          1, view_termnames,
          "$Rev$");

status
makeClassView(Class class)
{ declareClass(class, &view_decls);
  prependDelegateClass(class, NAME_editor);

  succeed;
}

