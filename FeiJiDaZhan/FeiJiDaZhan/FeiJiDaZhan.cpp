// FeiJiDaZhan.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <iostream>
#include <graphics.h> //easyX
#include <vector>
#include <conio.h>
using namespace std;

constexpr auto swidth = 600;
constexpr auto sheight = 780;

constexpr unsigned int SHP = 3;

constexpr auto hurttime = 1000; //ms

bool PointInRect(int x, int y, RECT& r)
{
    return (r.left <= x && x <= r.right && r.top <= y && y <= r.bottom);
}

bool RectDuangRect(RECT r1, RECT r2)
{
    RECT r;
    r.left = r1.left - (r2.right - r2.left);
    r.right = r1.right;
    r.top = r1.top - (r2.bottom - r2.top);
    r.bottom = r1.bottom;

    return (r.left < r2.left && r2.left <= r.right && r.top <= r2.top && r2.top <= r.bottom);
}

//一个开始界面
void Welcome()
{
    LPCTSTR title = _T("飞机大战");
    LPCTSTR tplay = _T("开始游戏");
    LPCTSTR texit = _T("退出游戏");

    RECT tplayr, texitr;
    BeginBatchDraw();
    setbkcolor(WHITE);
    cleardevice();
    settextstyle(60, 0, _T("黑体"));
    settextcolor(BLACK);
    outtextxy(swidth / 2 - textwidth(title) / 2, sheight / 5, title);

    settextstyle(40, 0, _T("黑体"));
    tplayr.left = swidth / 2 - textwidth(tplay) / 2;
    tplayr.right = tplayr.left + textwidth(tplay);
    tplayr.top = sheight / 5 * 2.5;
    tplayr.bottom = tplayr.top + textheight(tplay);

    texitr.left = swidth / 2 - textwidth(texit) / 2;
    texitr.right = texitr.left + textwidth(texit);
    texitr.top = sheight / 5 * 3;
    texitr.bottom = texitr.top + textheight(texit);

    outtextxy(tplayr.left, tplayr.top, tplay);
    outtextxy(texitr.left, texitr.top, texit);

    EndBatchDraw();

    while (true)
    {
        ExMessage mess;
        getmessage(&mess, EM_MOUSE);
        if (mess.lbutton)
        {
            if (PointInRect(mess.x, mess.y, tplayr))
            {
                return;
            }
            else if (PointInRect(mess.x, mess.y, texitr))
            {
                exit(0);
            }
        }
    }

    Sleep(100);
}

//结算页面
void Over(unsigned long long& kill)
{
    TCHAR* str = new TCHAR[128];
    _stprintf_s(str, 128, _T("击杀数: %llu"), kill);
    settextcolor(RED);
    outtextxy(swidth / 2 - textwidth(str) / 2, sheight / 5, str);

    //键盘事件（按Enter返回）
    LPCTSTR info = _T("按Enter返回");
    settextstyle(20, 0, _T("黑体"));
    outtextxy(swidth - textwidth(info), sheight - textheight(info), info);

    while (true)
    {
        ExMessage mess;
        getmessage(&mess, EM_KEY);
        if (mess.vkcode == 0x0D)
        {
            return;
        }
    }
}

// 背景，敌机，英雄，子弹
class BK
{
public:
    BK(IMAGE& img)
        :img(img), y(0)
    {

    }
    void Show()
    {   
        y += 4;
        if (y >= sheight) y = 0;

        putimage(0, y - sheight, &img, SRCCOPY);
        putimage(0, y, &img, SRCCOPY);
    }

private:
    IMAGE& img;
    int y;
};

class Hero
{
public:
    Hero(IMAGE& img)
        :img(img), HP(SHP)
    {
        rect.left = swidth / 2 - img.getwidth() / 2;
        rect.top = sheight - img.getheight();
        rect.right = rect.left + img.getwidth();
        rect.bottom = sheight;
    }

    void Show()
    {   
        setlinecolor(RED);
        setlinestyle(PS_SOLID, 6);
        putimage(rect.left, rect.top, &img);
        line(rect.left, rect.top - 5, rect.left + (img.getwidth() / SHP * HP), rect.top - 5);
    }

    void Control()
    {
        POINT pt;
        // 获取屏幕鼠标坐标
        GetCursorPos(&pt);
        // 转为窗口坐标
        ScreenToClient(GetHWnd(), &pt);
        
        // 给飞机矩形赋值
        rect.left = pt.x - img.getwidth() / 2;
        rect.top = pt.y - img.getheight() / 2;
        rect.right = rect.left + img.getwidth();
        rect.bottom = rect.top + img.getheight();

        // 限制飞机不出屏幕
        if (rect.left < 0) { rect.left = 0; rect.right = img.getwidth(); }
        if (rect.right > swidth) { rect.right = swidth; rect.left = swidth - img.getwidth(); }
        if (rect.top < 0) { rect.top = 0; rect.bottom = img.getheight(); }
        if (rect.bottom > sheight) { rect.bottom = sheight; rect.top = sheight - img.getheight(); }
    }

    bool hurt()
    {
        HP--;
        return (HP == 0) ? false : true;
    }

    RECT& GetRect() { return rect; }

private:
    IMAGE& img;
    RECT rect;

    unsigned int HP;
};

class Enemy
{
public:
    Enemy(IMAGE& img, int x, IMAGE*& boom)
        :img(img), isdie(false), boomsum(0)
    {
        selfboom[0] = boom[0];
        selfboom[1] = boom[1];
        selfboom[2] = boom[2];
        rect.left = x;
        rect.right = rect.left + img.getwidth();
        rect.top = -img.getheight();
        rect.bottom = 0;
    }

    bool Show()
    {
        if (isdie)
        {
            if (boomsum == 3)
            {
                return false;
            }
            putimage(rect.left, rect.top, selfboom + boomsum);
            boomsum++;

            return true;
        }

        if (rect.top >= sheight)
        {
            return false;
        }
        rect.top += 4;
        rect.bottom += 4;
        putimage(rect.left, rect.top, &img);

        return true;
    }

    void Isdie()
    {
        isdie = true;
    }

    RECT& GetRect() { return rect; }

private:
    IMAGE& img;
    RECT rect;
    IMAGE selfboom[3];

    bool isdie;
    int boomsum;
};

class Bullet
{
public:
    Bullet(IMAGE& img, RECT pr)
        :img(img)
    {
        rect.left = pr.left + (pr.right - pr.left) / 2 - img.getwidth() / 2;
        rect.right = rect.left + img.getwidth();
        rect.top = pr.top - img.getheight();
        rect.bottom = rect.top + img.getheight();
    }

    bool Show()
    {
        if (rect.bottom <= 0)
        {
            return false;
        }
        rect.top -= 3;
        rect.bottom -= 3;
        putimage(rect.left, rect.top, &img);

        return true;
    }

    RECT& GetRect() { return rect; }

protected:
    IMAGE& img;
    RECT rect;
};

class EBullet : public Bullet
{
public:
    EBullet(IMAGE& img, RECT pr)
        :Bullet(img, pr)
    {
        rect.left = pr.left + (pr.right - pr.left) / 2 - img.getwidth() / 2;
        rect.right = rect.left + img.getwidth();
        rect.top = pr.bottom;
        rect.bottom = rect.top + img.getheight();
    }

    bool Show()
    {
        if (rect.top >= sheight)
        {
            return false;
        }
        rect.top += 5;
        rect.bottom += 5;
        putimage(rect.left, rect.top, &img);

        return true;
    }
};

bool AddEnemy(vector<Enemy*>& es, IMAGE& enemyimg, IMAGE* boom)
{
    Enemy* e = new Enemy(enemyimg, abs(rand()) % (swidth - enemyimg.getwidth()), boom);
    for (auto& i : es)
    {
        if (RectDuangRect(i->GetRect(), e->GetRect()))
        {
            delete e;
            return false;
        }
    }
    es.push_back(e);
    return true;
}

bool Play()
{
    setbkcolor(WHITE);
    cleardevice();
    bool is_play = true;
    
    IMAGE heroimg, enemyimg, bkimg, bulletimg;
    IMAGE eboom[3];
    loadimage(&heroimg, _T("C:\\Users\\wang3\\Documents\\FeiJiDaZhan\\FeiJiDaZhan\\Resources\\me1.png"));
    loadimage(&enemyimg, _T("C:\\Users\\wang3\\Documents\\FeiJiDaZhan\\FeiJiDaZhan\\Resources\\enemy1.png"));
    loadimage(&bkimg, _T("C:\\Users\\wang3\\Documents\\FeiJiDaZhan\\FeiJiDaZhan\\Resources\\bk2.png"), swidth, sheight);
    loadimage(&bulletimg, _T("C:\\Users\\wang3\\Documents\\FeiJiDaZhan\\FeiJiDaZhan\\Resources\\bullet1.png"));

    loadimage(&eboom[0], _T("C:\\Users\\wang3\\Documents\\FeiJiDaZhan\\FeiJiDaZhan\\Resources\\enemy1_down1.png"));
    loadimage(&eboom[1], _T("C:\\Users\\wang3\\Documents\\FeiJiDaZhan\\FeiJiDaZhan\\Resources\\enemy1_down2.png"));
    loadimage(&eboom[2], _T("C:\\Users\\wang3\\Documents\\FeiJiDaZhan\\FeiJiDaZhan\\Resources\\enemy1_down3.png"));

    BK bk = BK(bkimg);
    Hero hp = Hero(heroimg);

    vector<Enemy*> es;
    vector<Bullet*> bs;
    vector<EBullet*> ebs;
    int bsing = 0;

    clock_t hurtlast = clock();

    unsigned long long kill = 0;

    for (int i = 0; i< 5;i++)
    {
        AddEnemy(es, enemyimg, eboom);
    }

    static bool paused = false;
    static bool spaceDown = false;

    while (is_play)
    {
        // 记录帧开始时间
        clock_t frame_start = clock();

        bsing++;
        if (bsing % 10 == 0)
        {
            bs.push_back(new Bullet(bulletimg, hp.GetRect()));
        }

        if (bsing == 60)
        {
            bsing = 0;
            for (auto& i : es)
            {
                ebs.push_back(new EBullet(bulletimg, i->GetRect()));
            }
        }

        //游戏暂停
        if (GetAsyncKeyState(VK_SPACE) & 0x8000)
        {
            if (!spaceDown)
            {
                paused = !paused; 
                spaceDown = true;
            }
        }
        else
        {
            spaceDown = false;
        }

        // ESC 退出游戏
        if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
        {
            is_play = false;
        }

        BeginBatchDraw();

        if (paused)
        {
            bk.Show();
            hp.Control();
            hp.Show();

            settextstyle(40, 0, _T("黑体"));
            settextcolor(RED);
            outtextxy(swidth / 2 - 80, sheight / 2, _T("游戏暂停"));

            EndBatchDraw();

            // 控制帧率
            clock_t frame_end = clock();
            int delay = 16 - int((frame_end - frame_start) * 1000 / CLOCKS_PER_SEC);
            if (delay > 0) Sleep(delay);
            continue;
        }

        bk.Show();
        hp.Control();
        hp.Show();

        // 子弹显示和删除
        for (auto it = bs.begin(); it != bs.end();)
        {
            if (!(*it)->Show())
            {
                delete* it;
                it = bs.erase(it);
            }
            else
            {
                ++it;
            }
        }

        // 敌机子弹显示和碰撞
        for (auto it = ebs.begin(); it != ebs.end();)
        {
            if (!(*it)->Show())
            {
                delete* it;
                it = ebs.erase(it);
            }
            else
            {
                if (RectDuangRect((*it)->GetRect(), hp.GetRect()))
                {
                    if (clock() - hurtlast >= hurttime)
                    {
                        is_play = hp.hurt();
                        hurtlast = clock();
                    }
                }
                ++it;
            }
        }

        // 敌机显示和碰撞
        for (auto it = es.begin(); it != es.end();)
        {
            if (RectDuangRect((*it)->GetRect(), hp.GetRect()))
            {
                if (clock() - hurtlast >= hurttime)
                {
                    is_play = hp.hurt();
                    hurtlast = clock();
                }
            }

            bool hit = false;
            for (auto bit = bs.begin(); bit != bs.end(); ++bit)
            {
                if (RectDuangRect((*bit)->GetRect(), (*it)->GetRect()))
                {
                    (*it)->Isdie();
                    delete* bit;
                    bs.erase(bit);
                    kill++;
                    hit = true;
                    break;
                }
            }

            if (!(*it)->Show())
            {
                delete* it;
                it = es.erase(it);
            }
            else
            {
                if (!hit) ++it;
            }
        }

        // 保持敌机数量
        for (int i = 0; i < 5 - es.size(); i++)
            AddEnemy(es, enemyimg, eboom);

        EndBatchDraw();

        // 帧率控制
        clock_t frame_end = clock();
        int delay = 16 - int((frame_end - frame_start) * 1000 / CLOCKS_PER_SEC);
        if (delay > 0) Sleep(delay);
    }

    // 清理内存
    for (auto b : bs) delete b;
    for (auto eb : ebs) delete eb;
    for (auto e : es) delete e;

    Over(kill);
    return true;
}

int main()
{
    // easyX初始化
    initgraph(swidth, sheight, EW_NOMINIMIZE | EW_SHOWCONSOLE);
    bool is_live = true;
    while (is_live)
    {
        Welcome();
        // Play
        is_live = Play();

    }

    return 0;
}
