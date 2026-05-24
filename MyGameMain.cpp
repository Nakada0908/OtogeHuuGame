#include "MyGameMain.h"

//ゲーム情報
DG::Image::SP imgT,imgA,imgO,imgR;
XI::GamePad::SP in1;
DG::Font::SP fontA;

int score;

enum class State{Normal,Hit,Non};
//構造体の定義
struct Target {
	State state;
	int x;
	int y;
	ML::Box2D hitBase;
	int timeCnt;
	int score;
	int moveX;
	int moveY;
	//追加
	int retu;//480/60で8列分の的が落ちてくる
	int playtimeCnt;//プレイ時間
	int playtimeMax;//終了するときの時間
};
//すべてのターゲットを1つの配列にまとめて宣言
Target targets[20];

//合わせるところ(下の判定部分)
struct Awase {
	ML::Box2D hitBase;
	int aretu;
};
Awase  atargets[8];
//押すところのキー表示
ML::Box2D osutoko;

//リザルト画面
ML::Box2D rizaruto;


void Target_Invalidate(Target& t_);//初期の無効化状態
void Target_Initialize(Target& t_);//再配置時の初期化
void Target_Draw(Target& t_);
void Target_UpDate(Target& t_);
int Target_HitCheck(Target& t_,Awase& a_);

void Awase_Initialize(Awase& a_, int index);
void Awase_Draw(Awase& a_);

//-----------------------------------------------------------------------------
//初期化処理
//機能概要：プログラム起動時に１回実行される（素材などの準備を行う）
//-----------------------------------------------------------------------------
void  MyGameMain_Initialize()
{
	srand((unsigned)time(NULL));//ランダムにする

	imgT = DG::Image::Create("./data/image/Target.png");
	imgA = DG::Image::Create("./data/image/Awase.png");
	imgO = DG::Image::Create("./data/image/Osutoko.png");
	imgR = DG::Image::Create("./data/image/rizaruto.png");
	score = 0;

	//キーボードの入力を受け取るオブジェクトを作成する
	//アナログスティック
	XI::AnalogAxisKB Is = { DIK_LEFT,DIK_RIGHT,DIK_UP,DIK_DOWN };
	XI::AnalogAxisKB rs = { DIK_NUMPAD4,DIK_NUMPAD6,DIK_NUMPAD8,DIK_NUMPAD2 };
	//トリガー
	XI::AnalogTriggerKB tg = { DIK_E,DIK_R };
	//十字キーとボタン
	XI::KeyDatas_KB key = {
		{DIK_A,XI::VGP::B1},{DIK_S,XI::VGP::B2},
		{DIK_D,XI::VGP::B3},{DIK_F,XI::VGP::B4},//A,S,D,Fキー
		{DIK_G,XI::VGP::L1},{DIK_H,XI::VGP::R1},
		{DIK_J,XI::VGP::L3},{DIK_K,XI::VGP::R3},//G,H,J,Kキー
		{DIK_SPACE,XI::VGP::HD}//スペースキー
	};
	in1 = XI::GamePad::CreateKB(Is, rs, tg, key);

	//まとの初期化ーーーーーーーーー
	for (int i = 0; i < 20; ++i) {
		Target_Invalidate(targets[i]);
	}

	//合わせるとこの初期化
	for (int i = 0; i < 8; ++i) {
		Awase_Initialize(atargets[i],i);
	}
	osutoko=ML::Box2D(0, 210, 480, 40);

	rizaruto = ML::Box2D(0, 0, 480, 270);

	fontA = DG::Font::Create("MS ゴシック", 24, 48);
}
//-----------------------------------------------------------------------------
//解放処理
//機能概要：プログラム終了時に１回実行される（素材などの解放を行う）
//-----------------------------------------------------------------------------
void  MyGameMain_Finalize( )
{
	imgT.reset();//意図的に解放する場合
	imgA.reset();
	imgO.reset();
	imgR.reset();
	fontA.reset();
}
//-----------------------------------------------------------------------------
//更新処理
//機能概要：ゲームの１フレームに当たる処理
//-----------------------------------------------------------------------------
void  MyGameMain_UpDate()
{
	auto inp = in1->GetState();
	//マウスクリックとまとの接触判定ーーーーー
	for (int i = 0; i < 20; ++i) 
		for (int a = 0; a < 8; ++a) {
			if (a == 0 && inp.B1.down) {//Aキーを押したら
				score += Target_HitCheck(targets[i], atargets[a]);
			}
			if (a == 1 && inp.B2.down) { //S
				score += Target_HitCheck(targets[i], atargets[a]);
			}
			if (a == 2 && inp.B3.down) { //D
				score += Target_HitCheck(targets[i], atargets[a]);
			}
			if (a == 3 && inp.B4.down) { //F
				score += Target_HitCheck(targets[i], atargets[a]);
			}
			if (a == 4 && inp.L1.down) {//G
				score += Target_HitCheck(targets[i], atargets[a]);
			}
			if (a == 5 && inp.R1.down){//H
				score += Target_HitCheck(targets[i], atargets[a]);
			}
			if (a == 6 && inp.L3.down) {//J
				score += Target_HitCheck(targets[i], atargets[a]);
			}
			if (a == 7 && inp.R3.down) {//K
				score += Target_HitCheck(targets[i], atargets[a]);
			}
		}
	
	//まとの行動処理
	for (int i = 0; i < 20; ++i) {
		Target_UpDate(targets[i]);
	}

	//スペースキーを押すとリセットされるようにする
	if (inp.HD.down) {//スペースキー
		for (int i = 0; i < 20; ++i) {
			Target_Invalidate(targets[i]);
		}
	}
}
//-----------------------------------------------------------------------------
//描画処理
//機能概要：ゲームの１フレームに当たる表示処理 ２Ｄ
//-----------------------------------------------------------------------------
void  MyGameMain_Render2D( )
{
	//まとの表示ーーーーー
	for (int i = 0; i < 20; ++i) {
		Target_Draw(targets[i]);
	}

	//合わせ部分の表示
	for (int i = 0; i < 8; ++i) {
		Awase_Draw(atargets[i]);
	}

	//押すとこの表示
	ML::Box2D draw = osutoko;
	ML::Box2D src (0, 0, 480, 40);
	imgO->Draw(draw, src);

	//マウスカーソルの座標を表示
	ML::Box2D textBox(0, 0, 480, 48);
	string text ="得点:"+to_string(score);
	fontA->Draw(textBox, text, ML::Color(1.0, 1.0f, 0.0f, 0.0f));

	//リザルトの表示
	if (targets[0].playtimeCnt >= targets[0].playtimeMax) {
		ML::Box2D draw = rizaruto;
		ML::Box2D src(0, 0, 480, 270);
		imgR->Draw(draw, src);
	}
}




void Target_Invalidate(Target& t_)//初期の無効化状態
{
	t_.state = State::Normal;
	t_.timeCnt = rand() % 300;
	t_.x = -1000;
	t_.y = 0;
	t_.moveX = 0;
	t_.moveY = 0;
	t_.hitBase = ML::Box2D(0, 0, 60, 40);
	t_.score = 100;
	//追加
	t_.retu = rand() % 8;
	t_.playtimeCnt = 0;
	t_.playtimeMax = 60 *60;//60フレーム*秒数
}

void Target_Initialize(Target& t_)//再配置時の初期化
{
	t_.state = State::Normal;
	int side = rand() % 1;
	switch (side) {
	case 0://上から
		t_.x = t_.retu * 60;
		t_.y = -40;
		t_.moveX = 0;
		t_.moveY = +3;//落ちてくる速度
		break;
	}
}

void Awase_Initialize(Awase& a_, int index)
{
	a_.aretu = 8;//480/60＝8
	a_.hitBase = ML::Box2D(index * 60, 210, 60, 40);
}

void Target_Draw(Target& t_)
{
	if (t_.state != State::Non) {
		ML::Box2D draw = t_.hitBase;
		ML::Box2D src(0, 0, 10, 10);//初期値はダミーデータ
		if (t_.state == State::Normal) { src = ML::Box2D(0, 0, 60, 40); }
		if (t_.state == State::Hit) { src = ML::Box2D(60, 0, 60, 40); }
		draw.x += t_.x;
		draw.y += t_.y;
		imgT->Draw(draw, src);//画像を描画
	}
}

void Awase_Draw(Awase& a_)
{
	ML::Box2D draw = a_.hitBase;
	ML::Box2D src(0, 0, 60, 40);
	imgA->Draw(draw, src);//画像を描画
}

void Target_UpDate(Target& t_)
{
	if (t_.playtimeCnt <= t_.playtimeMax) {
		if (t_.state != State::Non) {
			t_.timeCnt++;
			t_.playtimeCnt++;
			//寿命終了時の処理
			if (t_.timeCnt >= 300) {
				t_.timeCnt = 0;
				Target_Initialize(t_);//再配置
			}
			//通常時の処理
			t_.x += t_.moveX;
			t_.y += t_.moveY;
		}
	}
	else{//時間になったら無効化する
		t_.state = State::Hit;
	}
}

int Target_HitCheck(Target& t_,Awase& a_)
{
	if (t_.state == State::Hit) {}
		else {
			ML::Box2D hit = t_.hitBase;
			ML::Box2D ahit = a_.hitBase;
			hit.x += t_.x;
			hit.y += t_.y;
			if (hit.x <= ahit.x && ahit.x < hit.x + hit.w &&
				hit.y <= ahit.y && ahit.y < hit.y + hit.h)
				{
				t_.state = State::Hit;
				 return t_.score;
			}
		}
	return 0;
}
