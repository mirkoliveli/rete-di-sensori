//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit1.h"
#include <dstring.h>
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
int Acquisisci = 0;

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button1Click(TObject *Sender)
{
	 if(Acquisisci == 0)
	 {
		Acquisisci = 1;
		Button1->Caption = "Stop";
	 }
	 else
     {
		Acquisisci = 0;
		Button1->Caption = "Acquisisci";
	 }
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
	if(Acquisisci == 1)
	{
        TByteDynArray t;
		float temp;
		t.set_length(2);
		IdTCPClient1->Port  = StrToInt(EditPort->Text.c_str());
		IdTCPClient1->Host = EditHost->Text;
		IdTCPClient1->ConnectTimeout = 1000;
		// Sensore 1
		IdTCPClient1->Connect();
		//if(!IdTCPClient1->Connected()) return;
		IdTCPClient1->IOHandler->Write((byte)1);
		IdTCPClient1->Socket->ReadBytes(t, 2, false);
		IdTCPClient1->Disconnect();
		temp = ((t[0]<<8) | t[1]) / 256.0;
		Label4->Caption = FloatToStrF(temp, ffFixed, 7, 1);
        // Sensore 2
		IdTCPClient1->Connect();
		IdTCPClient1->IOHandler->Write((byte)2);
		IdTCPClient1->Socket->ReadBytes(t, 2, false);
		IdTCPClient1->Disconnect();
		temp = ((t[0]<<8) | t[1]) / 256.0;
		Label5->Caption = FloatToStrF(temp, ffFixed, 7, 1);
	}
}
//---------------------------------------------------------------------------

