//Dialog.cpp
#ifndef __DIALOG_CPP_
#define __DIALOG_CPP_

#include "Dialog.h"
#include "TextNavigate.h"

/*******************************************************************************
              class CConfigDialog
*******************************************************************************/

CConfigDialog::CConfigDialog(int NumElements, PFDialogItem Elements, int FNumElements)
  : CRefArray<FarDialogItem>(NumElements)
{
  Setup(0, Elements, (-1 == FNumElements) ? NumElements : FNumElements);
}

void CConfigDialog::Setup(int From, PFDialogItem p, int ECount)
{
  for (int n = 0; n < ECount; n++)
    Setup(From+n, p[n]);
}

PFarDialogItem CConfigDialog::Setup(int num, const FDialogItem& it)
{
  PFarDialogItem p = Item(num);
  if (!p) return NULL;

  p->Type = (int)(it.Type & 0xFFFF);
  p->X1 = it.X1;
  p->Y1 = it.Y1;
  p->X2 = it.X2;
  p->Y2 = it.Y2;
  p->Focus = IS_FLAG(it.Type, FFDI_FOCUSED);
  p->Selected = IS_FLAG(it.Type, FFDI_SELECTED);
  p->Flags = it.Flags;
  p->DefaultButton = IS_FLAG(it.Type, FFDI_DEFAULT);
  lstrcpy(p->Data, it.Text ? it.Text : "");

  return p;
}

int CConfigDialog::Execute(int w, int h, CONSTSTR Help)
{
  return Info.Dialog(Info.ModuleNumber, -1, -1, w, h,
                     (char *)Help,
                     (PFarDialogItem)Items(),
                     Count());
}

#endif
