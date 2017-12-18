//---------------------------------------------------------------------------

#ifndef Unit1H
#define Unit1H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <FMX.Controls.hpp>
#include <FMX.Forms.hpp>
#include <FMX.Controls.Presentation.hpp>
#include <FMX.StdCtrls.hpp>
#include <FMX.Types.hpp>
#include <FMX.Ani.hpp>
#include <FMX.Layouts.hpp>
#include <FMX.Media.hpp>
#include <System.IOUtils.hpp>
#include <functional>
//---------------------------------------------------------------------------
class TGameForm : public TForm
{
__published:	// IDE で管理されるコンポーネント
	TPanel *Panel1;
	TPanel *Court;
	TPanel *Panel2;
	TPanel *ScreenPanel;
	TPanel *RacketPanel;
	TSpeedButton *StartButton;
	TSpeedButton *StopButton;
	TLabel *Racket;
	TLabel *Racket1;
	TLabel *Racket2;
	TLabel *Racket3;
	TLabel *Racket4;
	TStyleBook *StyleBook1;
	TLabel *Ball;
	TLabel *ScoreTitle;
	TLabel *ScoreLabel;
	TLayout *Layout1;
	TSpeedButton *LeftButton;
	TSpeedButton *RightButton;
	TFloatAnimation *FloatAnimation1;
	TTimer *Timer1;
	void __fastcall LeftButtonClick(TObject *Sender);
	void __fastcall RightButtonClick(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
	void __fastcall StartButtonClick(TObject *Sender);
	void __fastcall StopButtonClick(TObject *Sender);
private:	// ユーザー宣言
	static constexpr const wchar_t* racket_mp3	{L"racket.mp3"};
	static constexpr const wchar_t* wall_mp3	{L"wall.mp3"};
	int FAng, FDeltaX, FDeltaY;
	int FScore, FRacket;
	double FSpeed{15};
	void __fastcall SetScore(const int NewScore);
	void __fastcall SetRacketCount(const int NewRacket);
	void __fastcall MoveBall();         //ボールを動かす関数
	void __fastcall CalcNewDelta();     //FDeltaX、FDeltaYを再計算する関数
	bool __fastcall CheckHit(const int ox, const int oy, int &nx, int &ny, const TPosition *r);
	TMediaPlayer* __fastcall beep_start(const String& fname);
	void __fastcall LostRacket(std::function<void()> func1);
	void __fastcall GameOver();
public:		// ユーザー宣言
	__fastcall TGameForm(TComponent* Owner);
	__property int Score = { read = FScore, write = SetScore };
 	__property int RacketCount = { read = FRacket, write = SetRacketCount };
};
//---------------------------------------------------------------------------
extern PACKAGE TGameForm *GameForm;
//---------------------------------------------------------------------------
#endif
