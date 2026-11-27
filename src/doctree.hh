#ifndef __DOCTREE_HH__
#define __DOCTREE_HH__

#include "lout/misc.hh"

class DoctreeNode {
   public:
      DoctreeNode *parent;
      DoctreeNode *sibling;
      DoctreeNode *lastChild;
      int num; // unique ascending id
      int element;
      lout::misc::SimpleVector<char*> *klass;
      const char *pseudo;
      const char *id;
      /* Dynamic pseudo-class state (:hover, :active), independent of
       * the static "pseudo" string (:link/:visited/:first-child), so
       * that e.g. a hovered link still matches :link rules. */
      bool hovered;
      bool active;

      DoctreeNode () {
         parent = NULL;
         sibling = NULL;
         lastChild = NULL;
         klass = NULL;
         pseudo = NULL;
         id = NULL;
         element = 0;
         hovered = false;
         active = false;
      };

      ~DoctreeNode () {
         dFree ((void*) id);
         while (lastChild) {
            DoctreeNode *n = lastChild;
            lastChild = lastChild->sibling;
            delete n;
         }
         if (klass) {
            for (int i = 0; i < klass->size (); i++)
               dFree (klass->get(i));
            delete klass;
         }
      }
};

/**
 * \brief HTML document tree interface.
 *
 * The Doctree class defines the interface to the parsed HTML document tree
 * as it is used for CSS selector matching.
 */
class Doctree {
   private:
      DoctreeNode *topNode;
      DoctreeNode *rootNode;
      int num;

   public:
      Doctree () {
         rootNode = new DoctreeNode;
         topNode = rootNode;
         num = 0;
      };

      ~Doctree () {
         delete rootNode;
      };

      DoctreeNode *push () {
         DoctreeNode *dn = new DoctreeNode ();
         dn->parent = topNode;
         dn->sibling = dn->parent->lastChild;
         
         // Handle :first-child pseudo-class
         DoctreeNode *oldLastChild = dn->parent->lastChild;
         if (oldLastChild == NULL) {
            // This is the first child
            dn->pseudo = "first-child";
         } else {
            // There was already a first child - find and unmark it
            // The first child is the one at the end of the sibling chain (sibling == NULL)
            DoctreeNode *firstChild = oldLastChild;
            while (firstChild->sibling != NULL) {
               firstChild = firstChild->sibling;
            }
            firstChild->pseudo = NULL;
         }
         
         dn->parent->lastChild = dn;
         dn->num = num++;
         topNode = dn;
         return dn;
      };

      void pop () {
         assert (topNode != rootNode); // never pop the root node
         topNode = topNode->parent;
      };

      inline DoctreeNode *top () {
         if (topNode != rootNode)
            return topNode;
         else
            return NULL;
      };

      inline DoctreeNode *parent (const DoctreeNode *node) {
         if (node->parent != rootNode)
            return node->parent;
         else
            return NULL;
      };

      inline DoctreeNode *sibling (const DoctreeNode *node) {
         return node->sibling;
      };
};

#endif
