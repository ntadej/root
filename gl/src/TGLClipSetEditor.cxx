// @(#)root/gl:$Id$
// Author:  Matevz Tadel, Jun 2007

/*************************************************************************
 * Copyright (C) 1995-2004, Rene Brun and Fons Rademakers.               *
 * All rights reserved.                                                  *
 *                                                                       *
 * For the licensing terms see $ROOTSYS/LICENSE.                         *
 * For the list of contributors see $ROOTSYS/README/CREDITS.             *
 *************************************************************************/

#include "TGLClipSetEditor.h"
#include "TGLClip.h"
#include "TVirtualPad.h"
#include "TColor.h"
#include "TGLabel.h"
#include "TGButton.h"
#include "TGButtonGroup.h"
#include "TGNumberEntry.h"


//______________________________________________________________________________
// TGLClipSetSubEditor
//
//

ClassImp(TGLClipSetSubEditor)


//______________________________________________________________________________
TGLClipSetSubEditor::TGLClipSetSubEditor(const TGWindow *p) :
   TGVerticalFrame(p),
   fM(0),
   fCurrentClip(kClipNone),
   fTypeButtons(0),
   fPlanePropFrame(0),
   fPlaneProp(),
   fBoxPropFrame(0),
   fBoxProp(),
   fClipInside(0),
   fClipEdit(0),
   fClipShow(0),
   fApplyButton(0)
{
   // Constructor.

   fTypeButtons = new TGButtonGroup(this, "Clip Type");
   new TGRadioButton(fTypeButtons, "None");
   new TGRadioButton(fTypeButtons, "Plane");
   new TGRadioButton(fTypeButtons, "Box");

   AddFrame(fTypeButtons, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 3, 3, 3, 3));
   // Clip inside / edit
   fClipInside = new TGCheckButton(this, "Clip away inside");
   AddFrame(fClipInside, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 3, 3, 3, 3));
   fClipEdit   = new TGCheckButton(this, "Edit In Viewer");
   AddFrame(fClipEdit, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 3, 3, 3, 3));
   fClipShow   = new TGCheckButton(this, "Show In Viewer");
   AddFrame(fClipShow, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 3, 3, 3, 3));

   // Plane properties
   fPlanePropFrame = new TGCompositeFrame(this);
   //fPlanePropFrame->SetCleanup(kDeepCleanup);
   AddFrame(fPlanePropFrame, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 3, 3, 3, 3));

   static const char * const planeStr[] = { "aX + ", "bY +", "cZ + ", "d = 0" };

   for (Int_t i = 0; i < 4; ++i) {
      TGLabel *label = new TGLabel(fPlanePropFrame, planeStr[i]);
      fPlanePropFrame->AddFrame(label, new TGLayoutHints(kLHintsTop | kLHintsLeft, 3, 3, 3, 3));
      fPlaneProp[i] = new TGNumberEntry(fPlanePropFrame, 1., 8);
      fPlanePropFrame->AddFrame(fPlaneProp[i], new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 3, 3, 3, 3));
   }

   // Box properties
   fBoxPropFrame = new TGCompositeFrame(this);
   AddFrame(fBoxPropFrame, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 3, 3, 3, 3));

   static const char * const boxStr[] = {"Center X", "Center Y", "Center Z", "Length X", "Length Y", "Length Z" };

   for (Int_t i = 0; i < 6; ++i) {
      TGLabel *label = new TGLabel(fBoxPropFrame, boxStr[i]);
      fBoxPropFrame->AddFrame(label, new TGLayoutHints(kLHintsTop | kLHintsLeft, 3, 3, 3, 3));
      fBoxProp[i] = new TGNumberEntry(fBoxPropFrame, 1., 8);
      fBoxPropFrame->AddFrame(fBoxProp[i], new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 3, 3, 3, 3));
   }

   // Apply button
   fApplyButton = new TGTextButton(this, "Apply");
   AddFrame(fApplyButton, new TGLayoutHints(kLHintsTop | kLHintsCenterX | kLHintsExpandX, 3, 3, 3, 3));

   fTypeButtons->Connect("Clicked(Int_t)", "TGLClipSetSubEditor", this, "ClipTypeChanged(Int_t)");
   fClipInside->Connect("Clicked()", "TGLClipSetSubEditor", this, "UpdateViewerClip()");
   fClipEdit->Connect("Clicked()", "TGLClipSetSubEditor", this, "UpdateViewerClip()");
   fClipShow->Connect("Clicked()", "TGLClipSetSubEditor", this, "UpdateViewerClip()");

   for (Int_t i = 0; i < 4; ++i)
      fPlaneProp[i]->Connect("ValueSet(Long_t)", "TGLClipSetSubEditor", this, "ClipValueChanged()");

   for (Int_t i = 0; i < 6; ++i)
      fBoxProp[i]->Connect("ValueSet(Long_t)", "TGLClipSetSubEditor", this, "ClipValueChanged()");

   fApplyButton->Connect("Pressed()", "TGLClipSetSubEditor", this, "UpdateViewerClip()");
}


//______________________________________________________________________________
void TGLClipSetSubEditor::SetModel(TGLClipSet* m)
{
   // Set model object.

   fM = m;

   Double_t clip[6] = {0.};

   fM->GetClipState(fCurrentClip, clip);

   fApplyButton->SetState(kButtonDisabled);

   // Button ids run from 1
   if (TGButton *btn = fTypeButtons->GetButton(fCurrentClip+1)) {
      btn->SetDown();
      fTypeButtons->SetButton(fCurrentClip+1);
   }
   Bool_t active = (fCurrentClip != kClipNone);
   fClipInside->SetEnabled(active);
   fClipEdit  ->SetEnabled(active);
   fClipShow  ->SetEnabled(active);
   if (active) {
      fClipEdit->SetDown(fM->GetShowManip());
      fClipShow->SetDown(fM->GetShowClip());
      fClipInside->SetDown(fM->GetCurrentClip()->GetMode() == TGLClip::kInside);

      if (fCurrentClip == kClipPlane) {
         HideFrame(fBoxPropFrame);
         ShowFrame(fPlanePropFrame);
         for (Int_t i = 0; i < 4; ++i)
            fPlaneProp[i]->SetNumber(clip[i]);
      } else if (fCurrentClip == kClipBox) {
         HideFrame(fPlanePropFrame);
         ShowFrame(fBoxPropFrame);
         for (Int_t i = 0; i < 6; ++i)
            fBoxProp[i]->SetNumber(clip[i]);
      }
   } else {
      HideFrame(fPlanePropFrame);
      HideFrame(fBoxPropFrame);
   }
}


//______________________________________________________________________________
void TGLClipSetSubEditor::Changed()
{
   // Emit Changed signal.

   Emit("Changed()");
}


//______________________________________________________________________________
void TGLClipSetSubEditor::ClipValueChanged()
{
   // One of number entries was changed.

   fApplyButton->SetState(kButtonUp);
}


//______________________________________________________________________________
void TGLClipSetSubEditor::ClipTypeChanged(Int_t id)
{
   // Clip type radio button changed - update viewer.

   switch (id)
   {
      case 2:  fCurrentClip = kClipPlane; break;
      case 3:  fCurrentClip = kClipBox;   break;
      default: fCurrentClip = kClipNone;  break;
   }
   fM->SetClipType(fCurrentClip);
   SetModel(fM);
   ((TGMainFrame*)GetMainFrame())->Layout();
   Changed();
}


//______________________________________________________________________________
void TGLClipSetSubEditor::UpdateViewerClip()
{
   // Change clipping volume.

   Double_t data[6] = {0.};
   // Fetch GUI state for clip if 'type' into 'data' vector
   if (fCurrentClip == kClipPlane)
      for (Int_t i = 0; i < 4; ++i)
         data[i] = fPlaneProp[i]->GetNumber();
   else if (fCurrentClip == kClipBox)
      for (Int_t i = 0; i < 6; ++i)
         data[i] = fBoxProp[i]->GetNumber();

   fApplyButton->SetState(kButtonDisabled);

   fM->SetClipState(fCurrentClip, data);
   fM->SetShowManip(fClipEdit->IsDown());
   fM->SetShowClip (fClipShow->IsDown());
   if (fCurrentClip != kClipNone)
      fM->GetCurrentClip()->SetMode(fClipInside->IsDown() ? TGLClip::kInside : TGLClip::kOutside);

   Changed();
}


//______________________________________________________________________________
// TGLClipSetEditor
//
//

ClassImp(TGLClipSetEditor)


//______________________________________________________________________________
TGLClipSetEditor::TGLClipSetEditor(const TGWindow *p, Int_t width, Int_t height,
                                   UInt_t options, Pixel_t back) :
   TGedFrame(p, width, height, options | kVerticalFrame, back),
   fM  (0),
   fSE (0)
{
   // Constructor.

   MakeTitle("TGLClipSet");

   fSE = new TGLClipSetSubEditor(this);
   AddFrame(fSE, new TGLayoutHints(kLHintsTop, 2, 0, 2, 2));
   fSE->Connect("Changed()", "TGLClipSetEditor", this, "Update()");
}


//______________________________________________________________________________
void TGLClipSetEditor::SetModel(TObject* obj)
{
   // Set model object.

   fM = dynamic_cast<TGLClipSet*>(obj);
   fSE->SetModel(fM);
}
