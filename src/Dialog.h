//Dialog.h
#ifndef __DIALOG_H
#define __DIALOG_H

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "plugin.h"
#include "crtdll.h"
#include "commons.h"

#define IS_FLAG(val, flag) (((val)&(flag)) == (flag))
typedef const char *CONSTSTR;

#define FFDI_FOCUSED   0x00010000UL
#define FFDI_SELECTED  0x00020000UL
#define FFDI_DEFAULT   0x00040000UL

typedef struct FDialogItem
{
  DWORD    Type;
  int      X1;
  int      Y1;
  int      X2;
  int      Y2;
  DWORD    Flags;
  CONSTSTR Text;
} *PFDialogItem;

typedef struct FarDialogItem *PFarDialogItem;

//описание произвольного обьекта диалога
#define FDI_CONTROL(tp, x, y, x1, y1, fl, txt)    { tp, x, y, x1, y1, fl, txt }
//описание двойной рамки вокруг диалога
#define FDI_DOUBLEBOX(h, w, txt)              FDI_CONTROL(DI_DOUBLEBOX, 3, 1, (h-4), (w-2), 0, txt)
//описание метки (неактивного текста)
#define FDI_LABEL(x, y, txt)                  FDI_CONTROL(DI_TEXT, x, y, 0, 0, 0, txt)
//описание разделительной черты
#define FDI_SEPARATOR(y)                      FDI_CONTROL(DI_TEXT, -1, y, 0, 0, DIF_SEPARATOR, NULL)
//описание цветной метки
#define FDI_COLORLABEL(x, y, clr, txt)        FDI_CONTROL(DI_TEXT, x, y, 0, 0, DIF_SETCOLOR|(clr), txt)
//описания поля редактирования
#define FDI_EDIT(x, y, x1)                    FDI_CONTROL(DI_EDIT, x, y, x1, y, 0, NULL)
//описание поля редактирования в ReadOnly режиме
#define FDI_DISABLEDEDIT(x, y, x1)            FDI_CONTROL(DI_EDIT, x, y, x1, y, DIF_DISABLE, NULL)
//описание переключателя
#define FDI_CHECK(x, y, txt)                  FDI_CONTROL(DI_CHECKBOX, x, y, 0, 0, 0, txt)
//описание элемента выбора
#define FDI_RADIO(x, y, txt)                  FDI_CONTROL(DI_RADIOBUTTON, x, y, 0, 0, 0, txt)
//описание первого элемента выбора
#define FDI_STARTRADIO(x, y, txt)             FDI_CONTROL(DI_RADIOBUTTON, x, y, 0, 0, DIF_GROUP, txt)
//описание кнопки
#define FDI_BUTTON(x, y, txt)                 FDI_CONTROL(DI_BUTTON, x, y, 0, 0, 0, txt)
//описание кнопки по умолчанию
#define FDI_DEFBUTTON(x, y, txt)              FDI_CONTROL(DI_BUTTON|FFDI_DEFAULT, x, y, 0, 0, 0, txt)

/*******************************************************************************
              class CConfigDialog
*******************************************************************************/

typedef class CConfigDialog *PConfigDialog;

class CConfigDialog : public CRefArray<FarDialogItem>
{
 public:
  //CConfigDialog(int NumElemens);
  CConfigDialog(int NumElemens, PFDialogItem Elements, int FNumElements = -1);
  PFarDialogItem Setup(int num, const FDialogItem &p);
  void Setup(int From, PFDialogItem p, int count);
  int Execute(int w, int h, CONSTSTR Help = NULL);
};

#endif /* __DIALOG_H */
