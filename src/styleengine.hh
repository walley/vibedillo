/*
 * File: styleengine.hh
 *
 * Copyright 2008-2009 Johannes Hofmann <Johannes.Hofmann@gmx.de>
 * Copyright 2024 Rodrigo Arias Mallo <rodarima@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 */

#ifndef __STYLEENGINE_HH__
#define __STYLEENGINE_HH__

class StyleEngine;

#include "dw/core.hh"
#include "doctree.hh"
#include "css.hh"
#include "cssparser.hh"

/**
 * \brief This class provides the glue between HTML parser and CSS subsystem.
 *
 * It maintains a document tree and creates and caches style objects for use
 * by the HTML parser.
 * The HTML parser in turn informs StyleEngine about opened or closed
 * HTML elements and their attributes via the startElement() / endElement()
 * methods.
 */
class StyleEngine {
   private:
      struct Node {
         CssPropertyList *styleAttrProperties;
         CssPropertyList *styleAttrPropertiesImportant;
         CssPropertyList *nonCssProperties;
         dw::core::style::Style *style;
         dw::core::style::Style *wordStyle;
         dw::core::style::Style *backgroundStyle;
         bool inheritBackgroundColor;
         bool displayNone;
         DoctreeNode *doctreeNode;
      };

      dw::core::Layout *layout;
      lout::misc::SimpleVector <Node> *stack;
      CssContext *cssContext;
      Doctree *doctree;
      int importDepth;
      float dpmm;
      float zoom;
      DilloUrl *pageUrl, *baseUrl;

      void stackPush ();
      void stackPop ();
      void buildUserStyle ();
      dw::core::style::Style *style0 (int i, BrowserWindow *bw);
      dw::core::style::Style *wordStyle0 (BrowserWindow *bw);
      dw::core::style::Style *pseudoElementStyle0 (int i, BrowserWindow *bw,
                                                   const char *pseudo);
      inline void setNonCssHint(CssPropertyName name, CssValueType type,
                                CssPropertyValue value) {
         Node *n = stack->getRef (stack->size () - 1);

         if (!n->nonCssProperties)
            n->nonCssProperties = new CssPropertyList (true);
         n->nonCssProperties->set(name, type, value);
      }
      void preprocessAttrs (dw::core::style::StyleAttrs *attrs);
      void postprocessAttrs (dw::core::style::StyleAttrs *attrs);
      void apply (int i, dw::core::style::StyleAttrs *attrs,
                  CssPropertyList *props, BrowserWindow *bw,
                  dw::core::style::Font *referenceFont = NULL);
      void computeBorderWidth (int *dest, CssProperty *p,
                               dw::core::style::Font *font);

   public:
      static void init ();
      bool computeValue (int *dest, CssLength value,
                         dw::core::style::Font *font);
      bool computeValue (int *dest, CssLength value,
                         dw::core::style::Font *font, int percentageBase);
      bool computeLength (dw::core::style::Length *dest, CssLength value,
                          dw::core::style::Font *font);

      StyleEngine (dw::core::Layout *layout,
                   const DilloUrl *pageUrl, const DilloUrl *baseUrl, float zoom);
      ~StyleEngine ();

      void parse (DilloHtml *html, DilloUrl *url, const char *buf, int buflen,
                  CssOrigin origin, int sheetOrder = 0);
      void startElement (int tag, BrowserWindow *bw);
      void startElement (const char *tagname, BrowserWindow *bw);
      void setId (const char *id);
      const char * getId () { return doctree->top ()->id; };
      void setClass (const char *klass);
      void setStyle (const char *style);
      void endElement (int tag);
      void setPseudoLink ();
      void setPseudoVisited ();
      inline void setNonCssHint(CssPropertyName name, CssValueType type,
                                int value) {
         CssPropertyValue v;
         v.intVal = value;
         setNonCssHint (name, type, v);
      }
      inline void setNonCssHint(CssPropertyName name, CssValueType type,
                                const char *value) {
         CssPropertyValue v;
         v.strVal = dStrdup(value);
         setNonCssHint (name, type, v);
      }
      inline void setNonCssHint(CssPropertyName name, CssValueType type,
                                CssLength value) {
         CssPropertyValue v;
         v.lenVal = value;
         setNonCssHint (name, type, v);
      }
      void inheritNonCssHints ();
      void clearNonCssHints ();
      /* Compute the style of the current element as if it had the given
       * pseudo class ("before"/"after"), for generated content. Sets
       * *hasContent to true when a "content" property other than
       * none/normal matches. The returned style must be unref()ed. */
      dw::core::style::Style *pseudoStyle (BrowserWindow *bw,
                                           const char *pseudo,
                                           bool *hasContent);
      /* Compute the style of the current element (or of its parent) as if
       * it had the ":first-line" pseudo-element. Returns NULL when no
       * ":first-line" rule changes the style. The returned style must be
       * unref()ed. */
      dw::core::style::Style *firstLineStyle (BrowserWindow *bw);
      dw::core::style::Style *parentFirstLineStyle (BrowserWindow *bw);
      dw::core::style::Style *firstLetterStyle (BrowserWindow *bw);
      dw::core::style::Style *parentFirstLetterStyle (BrowserWindow *bw);
      void restyle (BrowserWindow *bw);
      void inheritBackgroundColor (); /* \todo get rid of this somehow */
      dw::core::style::Style *backgroundStyle (BrowserWindow *bw);
      dw::core::style::Color *backgroundColor ();
      dw::core::style::StyleImage *backgroundImage
         (dw::core::style::BackgroundRepeat *bgRepeat,
          dw::core::style::BackgroundAttachment *bgAttachment,
          dw::core::style::Length *bgPositionX,
          dw::core::style::Length *bgPositionY);

      inline dw::core::style::Style *style (BrowserWindow *bw) {
         dw::core::style::Style *s = stack->getRef (stack->size () - 1)->style;
         if (s)
            return s;
         else
            return style0 (stack->size () - 1, bw);
      };

      inline dw::core::style::Style *wordStyle (BrowserWindow *bw) {
         dw::core::style::Style *s = stack->getRef(stack->size()-1)->wordStyle;
         if (s)
            return s;
         else
            return wordStyle0 (bw);
      };
};

#endif
