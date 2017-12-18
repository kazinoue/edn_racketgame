//---------------------------------------------------------------------------

#include <fmx.h>
#pragma hdrstop

#include "Unit1.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.fmx"
TGameForm *GameForm;
//---------------------------------------------------------------------------
__fastcall TGameForm::TGameForm(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TGameForm::LeftButtonClick(TObject *Sender)
{
	//���P�b�g�����Ɉړ�
	if (Racket->Position->X > 0)
	{
		FloatAnimation1->StartValue = Racket->Position->X;
		FloatAnimation1->StopValue = Racket->Position->X -60;
		FloatAnimation1->Start();
	}
}

void __fastcall TGameForm::RightButtonClick(TObject *Sender)
{
	//���P�b�g���E�Ɉړ�
	if (Racket->Position->X <= (Court->Width - Racket->Width))
	{
		FloatAnimation1->StartValue = Racket->Position->X;
		FloatAnimation1->StopValue = Racket->Position->X +60;
		FloatAnimation1->Start();
	}
}
//---------------------------------------------------------------------------

void __fastcall TGameForm::Timer1Timer(TObject *Sender)
{
	MoveBall();
}
//---------------------------------------------------------------------------

void __fastcall TGameForm::SetScore(const int NewScore)
{
	if ((FScore = NewScore) < 0)
		FScore = 0;
	ScoreLabel->Text = Format(L"%0.9d", ARRAYOFCONST((FScore)) );
}

void __fastcall TGameForm::SetRacketCount(const int NewRacket)
{
	if (FRacket != NewRacket)
	{
		(NewRacket > 5)?FRacket = 5:FRacket = NewRacket;
		if (FRacket < 0) FRacket = 0;

		for (int i = 0; i < 4; i++)
		{
			dynamic_cast<TLabel*>(FindComponent("Racket" + IntToStr(i+1)))->Visible = (i < FRacket-1);
		}
	}
}
void __fastcall TGameForm::CalcNewDelta()
{
	// �V�����p�x�̃��W�A�������߂�
	double rad = ((double)FAng / 180.0) * M_PI;
	FDeltaX = (int)(cos(rad) * FSpeed);  // X�̈ړ���
	FDeltaY = (int)(sin(rad) * FSpeed);  // Y�̈ړ���
}
void __fastcall TGameForm::MoveBall()
{
	// �ړ���̃{�[���̈ʒu���v�Z���܂�
	int x = Ball->Position->X + FDeltaX;
	int y = Ball->Position->Y + FDeltaY;

	TPosition *pos = Racket->Position; // ���P�b�g�̍��W
	auto f1 = [this](const int inp, const int in_ang, String mp3name)->int
	{
		auto beep = beep_start(mp3name);
		FAng = in_ang - FAng;  	// ���˕Ԃ����p�x���v�Z
		CalcNewDelta();  		// �V�����ړ��ʂ��v�Z
		if (beep != nullptr) {
			beep->Play();
		}
		if (inp < 0){ return 0; };
		return inp;
	};

   // ���P�b�g�Ƀq�b�g�������ǂ����𔻒�
	if (CheckHit(Ball->Position->X, Ball->Position->Y, x, y, pos)) {
		if (x < pos->X + 2 || x + Ball->Width > pos->X + Ball->Width - 2)
		{
			f1(0, 360, racket_mp3);
			FAng += (Random(60) - 30);  // �p�ň���������
		}
		else
			f1(0, 360, racket_mp3);
		++Score;
		Ball->Position->X = x;
		Ball->Position->Y = y;
		return;
	}
	//�ǂɓ����������̔���
	if (y + Ball->Height >= Court->Height)
	{
		// ���̕�
		// �ǂɓ�������
		Timer1->Enabled = false;
		LostRacket(  	// ���P�b�g������
		[this]()
		{
			RacketCount--;
			if (RacketCount == 0)
			{
				GameOver();  // �Q�[���I�[�o�[
			}
			else
			{
				// ���̃��P�b�g���g�����Q�[���̏���
				FSpeed += 0.4;
				Ball->Position->X = Court->Width / 2;
				Ball->Position->Y = Court->Height / 3;
				FAng = Random(45) + 90;
				CalcNewDelta();
				Timer1->Enabled = true;
			}
		});
		return;
	}
	else if (y < 0)
	{
		// ��̕�
		FSpeed += 0.1;		// ��̕ǂɓ�����Ə��������Ȃ�܂�
		y = f1(y, 360, wall_mp3);
	}


	if (x + Ball->Width >= Court->Width)
	{
		// �E�̕�
		x = f1(Court->Width - Ball->Width, 182, wall_mp3);
	}
	else if (x < 0)
	{
		// ���̕�
		x = f1(x, 182, wall_mp3);
	}

	// �{�[���̈ʒu��ύX
	Ball->Position->X = x;
	Ball->Position->Y = y;
}

TMediaPlayer* __fastcall TGameForm::beep_start(const String& fname)
{
#if defined(_PLAT_IOS) || defined(_PLAT_ANDROID)
	auto mp = new TMediaPlayer(this);
	mp->FileName = System::Ioutils::TPath::GetDocumentsPath() +
			System::Ioutils::TPath::DirectorySeparatorChar + fname;
	return mp;
#else
    return nullptr;
#endif
}

void __fastcall TGameForm::LostRacket(std::function<void()> func1)
{
	// ���P�b�g�̑傫����ۊ�
	const int orgWidth	= Racket->Width;
	const int orgHeight = Racket->Height;
	TThread::CreateAnonymousThread([this, func1, orgWidth, orgHeight](){
		// ���P�b�g������������
		for (int i = 0, w = orgWidth/2; i < w; i++) {
			TThread::Synchronize(TThread::CurrentThread, [this, i, w, orgWidth, orgHeight](){
				Racket->Width = Racket->Width - 2;
				Racket->Height = (orgHeight * (w - i))/ w;
				Racket->Position->X = Racket->Position->X + 1;
			});
			Sleep(20);  // 20msec�҂�
		}
		Sleep(100);  // 100msec�҂�
		// ���P�b�g�����̑傫���ɖ߂�
		TThread::Synchronize(TThread::CurrentThread,[this, orgWidth, orgHeight, func1](){
			Racket->Width = orgWidth;
			Racket->Height = orgHeight;
			// ���P�b�g�𒆉��ɔz�u
			Racket->Position->X = (Court->Width - orgWidth) / 2;
            func1();
		});
	})->Start();
}

void __fastcall TGameForm::GameOver()
{
   // �^�C�}�[�̒�~
	Timer1->Enabled = false;

	// �{�[�����\���ɂ���
    Ball->Visible = false;

    // �{�^���̏�ԕύX
    StartButton->Enabled = true;
	StopButton->Enabled = false;

	// �Q�[���I�[�o�[�̃��b�Z�[�W
	ShowMessage("Game Over");
}

bool __fastcall TGameForm::CheckHit(const int ox, const int oy, int &nx, int &ny, const TPosition *r)
{
	int ncx;
    if (oy < ny) {  // �{�[�������Ɍ������ē����Ă���Ƃ�
        // �{�[�������P�b�g�� Y���W��ʉ߂������H
        if (oy < r->Y && ny >= r->Y) {
            // ���P�b�g�ƃ{�[���̊�Ղ̌�_��X���W�����߂�
			ncx = ox + (int)((double)FDeltaX * (double)(oy - r->Y) / (double)(oy - ny));
            // ��_��X���W�����P�b�g�̋�`���ɂ��邩�H
            if (ncx > r->X - Ball->Width && ncx < r->X + Racket->Width) {
				// ��_�̍��W���ړ���̃{�[�����W�ɃZ�b�g
                nx = ncx;
                ny = oy + (int)((double)FDeltaY * (double)(oy - r->Y) / (double)(oy - ny));
                return true;
            }
        }
	}
    else {  // �{�[������Ɍ������ē����Ă���Ƃ�
        // �{�[�������P�b�g�� Y���W��ʉ߂������H
        if (oy < r->Y + Racket->Height && ny >= r->Y + Racket->Height) {
            // ���P�b�g�ƃ{�[���̊�Ղ̌�_��X���W�����߂�
			ncx = ox + (int)((double)FDeltaX * (double)(oy - (r->Y + Racket->Height))
                      / (double)(oy - ny));
            // ��_��X���W�����P�b�g�̋�`���ɂ��邩�H
            if (ncx > r->X - Ball->Width && ncx < r->X + Racket->Width) {
                // ��_�̍��W���ړ���̃{�[�����W�ɃZ�b�g
				nx = ncx;
                ny = oy + (int)((double)FDeltaY * (double)(oy - (r->Y + Racket->Height))
                      / (double)(oy - ny));
                return true;
			}
        }
	}
    return false;
}

void __fastcall TGameForm::StartButtonClick(TObject *Sender)
{
    // �����̏�����
    Randomize();

    // �{�[���̍ŏ��̊p�x��ݒ�
    FAng = Random(45) + 90;

    // �X�R�A�ƃ��P�b�g���̏�����
    Score = 0;
    RacketCount = 5;

    // �{�[���̏���
    FSpeed = 8.0;

    // �{�[���̍ŏ��̈ʒu
    Ball->Position->X = Court->Width / 2;
	Ball->Position->Y = Court->Height / 3;

    // �ړ��ʂ��v�Z
    CalcNewDelta();

    // �{�[����\��
    Ball->Visible = true;

    // �^�C�}�[�𓮍삳����
    Timer1->Enabled = true;

    // �{�^���̏�ԕύX
    StartButton->Enabled = false;
    StopButton->Enabled = true;

}
//---------------------------------------------------------------------------

void __fastcall TGameForm::StopButtonClick(TObject *Sender)
{
    // �^�C�}�[�̒�~
    Timer1->Enabled = false;

    // �{�^���̏�ԕύX
    StartButton->Enabled = true;
    StopButton->Enabled = false;
}
//---------------------------------------------------------------------------

