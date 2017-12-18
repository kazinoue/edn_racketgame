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
	//ラケットを左に移動
	if (Racket->Position->X > 0)
	{
		FloatAnimation1->StartValue = Racket->Position->X;
		FloatAnimation1->StopValue = Racket->Position->X -60;
		FloatAnimation1->Start();
	}
}

void __fastcall TGameForm::RightButtonClick(TObject *Sender)
{
	//ラケットを右に移動
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
	// 新しい角度のラジアンを求める
	double rad = ((double)FAng / 180.0) * M_PI;
	FDeltaX = (int)(cos(rad) * FSpeed);  // Xの移動量
	FDeltaY = (int)(sin(rad) * FSpeed);  // Yの移動量
}
void __fastcall TGameForm::MoveBall()
{
	// 移動後のボールの位置を計算します
	int x = Ball->Position->X + FDeltaX;
	int y = Ball->Position->Y + FDeltaY;

	TPosition *pos = Racket->Position; // ラケットの座標
	auto f1 = [this](const int inp, const int in_ang, String mp3name)->int
	{
		auto beep = beep_start(mp3name);
		FAng = in_ang - FAng;  	// 跳ね返った角度を計算
		CalcNewDelta();  		// 新しい移動量を計算
		if (beep != nullptr) {
			beep->Play();
		}
		if (inp < 0){ return 0; };
		return inp;
	};

   // ラケットにヒットしたかどうかを判定
	if (CheckHit(Ball->Position->X, Ball->Position->Y, x, y, pos)) {
		if (x < pos->X + 2 || x + Ball->Width > pos->X + Ball->Width - 2)
		{
			f1(0, 360, racket_mp3);
			FAng += (Random(60) - 30);  // 角で引っかけた
		}
		else
			f1(0, 360, racket_mp3);
		++Score;
		Ball->Position->X = x;
		Ball->Position->Y = y;
		return;
	}
	//壁に当たったかの判定
	if (y + Ball->Height >= Court->Height)
	{
		// 下の壁
		// 壁に当たった
		Timer1->Enabled = false;
		LostRacket(  	// ラケットを消す
		[this]()
		{
			RacketCount--;
			if (RacketCount == 0)
			{
				GameOver();  // ゲームオーバー
			}
			else
			{
				// 次のラケットを使ったゲームの準備
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
		// 上の壁
		FSpeed += 0.1;		// 上の壁に当たると少し早くなります
		y = f1(y, 360, wall_mp3);
	}


	if (x + Ball->Width >= Court->Width)
	{
		// 右の壁
		x = f1(Court->Width - Ball->Width, 182, wall_mp3);
	}
	else if (x < 0)
	{
		// 左の壁
		x = f1(x, 182, wall_mp3);
	}

	// ボールの位置を変更
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
	// ラケットの大きさを保管
	const int orgWidth	= Racket->Width;
	const int orgHeight = Racket->Height;
	TThread::CreateAnonymousThread([this, func1, orgWidth, orgHeight](){
		// ラケットを少しずつ消す
		for (int i = 0, w = orgWidth/2; i < w; i++) {
			TThread::Synchronize(TThread::CurrentThread, [this, i, w, orgWidth, orgHeight](){
				Racket->Width = Racket->Width - 2;
				Racket->Height = (orgHeight * (w - i))/ w;
				Racket->Position->X = Racket->Position->X + 1;
			});
			Sleep(20);  // 20msec待つ
		}
		Sleep(100);  // 100msec待つ
		// ラケットを元の大きさに戻す
		TThread::Synchronize(TThread::CurrentThread,[this, orgWidth, orgHeight, func1](){
			Racket->Width = orgWidth;
			Racket->Height = orgHeight;
			// ラケットを中央に配置
			Racket->Position->X = (Court->Width - orgWidth) / 2;
            func1();
		});
	})->Start();
}

void __fastcall TGameForm::GameOver()
{
   // タイマーの停止
	Timer1->Enabled = false;

	// ボールを非表示にする
    Ball->Visible = false;

    // ボタンの状態変更
    StartButton->Enabled = true;
	StopButton->Enabled = false;

	// ゲームオーバーのメッセージ
	ShowMessage("Game Over");
}

bool __fastcall TGameForm::CheckHit(const int ox, const int oy, int &nx, int &ny, const TPosition *r)
{
	int ncx;
    if (oy < ny) {  // ボールが下に向かって動いているとき
        // ボールがラケットの Y座標を通過したか？
        if (oy < r->Y && ny >= r->Y) {
            // ラケットとボールの奇跡の交点のX座標を求める
			ncx = ox + (int)((double)FDeltaX * (double)(oy - r->Y) / (double)(oy - ny));
            // 交点のX座標がラケットの矩形内にあるか？
            if (ncx > r->X - Ball->Width && ncx < r->X + Racket->Width) {
				// 交点の座標を移動後のボール座標にセット
                nx = ncx;
                ny = oy + (int)((double)FDeltaY * (double)(oy - r->Y) / (double)(oy - ny));
                return true;
            }
        }
	}
    else {  // ボールが上に向かって動いているとき
        // ボールがラケットの Y座標を通過したか？
        if (oy < r->Y + Racket->Height && ny >= r->Y + Racket->Height) {
            // ラケットとボールの奇跡の交点のX座標を求める
			ncx = ox + (int)((double)FDeltaX * (double)(oy - (r->Y + Racket->Height))
                      / (double)(oy - ny));
            // 交点のX座標がラケットの矩形内にあるか？
            if (ncx > r->X - Ball->Width && ncx < r->X + Racket->Width) {
                // 交点の座標を移動後のボール座標にセット
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
    // 乱数の初期化
    Randomize();

    // ボールの最初の角度を設定
    FAng = Random(45) + 90;

    // スコアとラケット数の初期化
    Score = 0;
    RacketCount = 5;

    // ボールの初速
    FSpeed = 8.0;

    // ボールの最初の位置
    Ball->Position->X = Court->Width / 2;
	Ball->Position->Y = Court->Height / 3;

    // 移動量を計算
    CalcNewDelta();

    // ボールを表示
    Ball->Visible = true;

    // タイマーを動作させる
    Timer1->Enabled = true;

    // ボタンの状態変更
    StartButton->Enabled = false;
    StopButton->Enabled = true;

}
//---------------------------------------------------------------------------

void __fastcall TGameForm::StopButtonClick(TObject *Sender)
{
    // タイマーの停止
    Timer1->Enabled = false;

    // ボタンの状態変更
    StartButton->Enabled = true;
    StopButton->Enabled = false;
}
//---------------------------------------------------------------------------

