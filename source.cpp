
#include <Windows.h>
#include <stdio.h>
#include <conio.h>
#include <time.h>
#include <vector>
#include <string>
#include <SHttp.h>
#include <algorithm/KMP.h>
using namespace std;
#pragma comment(lib, "ws2_32.lib")

void getpwd(char* pwd)
{
    auto hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    int n;
    int i = 0;
    while ((n = _getch()) != VK_RETURN)
    {
        if (n == VK_BACK)
        {
            if (i > 0)
            {
                COORD loc;
                CONSOLE_SCREEN_BUFFER_INFO csbi;
                GetConsoleScreenBufferInfo(hOutput, &csbi);
                loc = csbi.dwCursorPosition;
                --loc.X;
                SetConsoleCursorPosition(hOutput, loc);
                putchar(' ');
                SetConsoleCursorPosition(hOutput, loc);
                --i;
            }
        }
        else
        {
            if (i < 31)
            {
                putchar('*');
                pwd[i++] = n;
            }
        }
    }
    pwd[i] = 0;
    puts("");
}

const char node_time_strings[][7] = {
    "080000",
    "084500",
    "085500",
    "094000",
    "100000",
    "104500",
    "105500",
    "114000",
    "140000",
    "144500",
    "145500",
    "154000",
    "160000",
    "164500",
    "165500",
    "174000",
    "190000",
    "194500",
    "195500",
    "204000"
};

const char weekday_short_2[][3] = {
    "SU",
    "MO",
    "TU",
    "WE",
    "TH",
    "FR",
    "SA",
    "SU"
};

void insertEvent(FILE* fp,
                 const string& name,
                 const vector<string>& teachers,
                 const string& weeks,
                 const string& when,
                 const string& where)
{
    /*
      BEGIN:VEVENT
      DTSTART;TZID=Asia/Shanghai:20121009T140000
      DTEND;TZID=Asia/Shanghai:20121009T153500
      RRULE:FREQ=WEEKLY;COUNT=11;BYDAY=TU
      DTSTAMP:20130225T111338Z
      UID:5grm54mkqli1j260g1t0bpm88g@google.com
      CREATED:20121008T034303Z
      DESCRIPTION:周一平
      LAST-MODIFIED:20121008T035615Z
      LOCATION:A419
      SEQUENCE:0
      STATUS:CONFIRMED
      SUMMARY:大学物理II-2
      TRANSP:OPAQUE
      END:VEVENT
    */
    string teacher;
    auto itr = teachers.begin();
    teacher = move(*itr);
    for (++itr; itr != teachers.end(); ++itr)
    {
        teacher.append(",").append(*itr);
    }
    int start_week, end_week;
    const char* pweeks = weeks.c_str();
    while (true)
    {
        int t = sscanf(pweeks, "%d-%d", &start_week, &end_week);
        if (t < 1)
            break;
        if (t < 2)
            end_week = start_week;
        while (*pweeks)
            if (*pweeks++ == ',')
                break;
        tm first_day = {0};
        first_day.tm_year = 2013 - 1900;
        first_day.tm_mon  = 2 - 1;
        first_day.tm_mday = 24;
        auto time_base = mktime(&first_day);
        --start_week;
        int number_weeks;
        number_weeks = end_week - start_week;
        auto start_time = time_base + start_week * 7 * 24 * 60 * 60;
        const char* pstart;
        const char* pend;
        pstart = when.data();
        pend = pstart;
        while (*++pend)
            if (*pend == '[')
                break;
        string weekday = move(string(pstart, pend));
        int nWeekday;
        if (weekday == "日")
            nWeekday = 7;//0;
        else if (weekday == "一")
            nWeekday = 1;
        else if (weekday == "二")
            nWeekday = 2;
        else if (weekday == "三")
            nWeekday = 3;
        else if (weekday == "四")
            nWeekday = 4;
        else if (weekday == "五")
            nWeekday = 5;
        else if (weekday == "六")
            nWeekday = 6;
        start_time += nWeekday * 24 * 60 * 60;
        tm* start_day = localtime(&start_time);
        fprintf(fp,
                "BEGIN:VEVENT\n"
                "DTSTART;TZID=Asia/Shanghai:%d%02d%02dT",
                start_day->tm_year + 1900,
                start_day->tm_mon + 1,
                start_day->tm_mday);
        ++pend;
        int& start_node = start_week;
        int& end_node = end_week;
        sscanf(pend, "%d-%d", &start_node, &end_node);
        --start_node;
        --end_node;
        fprintf(fp, node_time_strings[start_node << 1]);
        fprintf(fp, "\nDTEND;TZID=Asia/Shanghai:%d%02d%02dT%s\n",
                start_day->tm_year + 1900,
                start_day->tm_mon + 1,
                start_day->tm_mday,
                node_time_strings[(end_node << 1) + 1]);
        if (number_weeks > 1)
        {
            fprintf(fp, "RRULE:FREQ=WEEKLY;COUNT=%d;BYDAY=%s\n",
                    number_weeks,
                    weekday_short_2[nWeekday]);
        }
        fprintf(fp,
                "DESCRIPTION:%s\n"
                "LOCATION:%s\n"
                "STATUS:CONFIRMED\n"
                "SUMMARY:%s\n"
                "TRANSP:OPAQUE\n"
                "END:VEVENT\n",
                teacher.c_str(),
                where.c_str(),
                name.c_str());
    }
}

void translate(const char* buf)
{
    const char *pstart;
    const char *pend;
    string      name;
    string      teacher;
    vector<string> teachers;
    string      weeks;
    string      when;
    string      where;
    FILE*       fp = fopen("classinfo.ics", "wb");
    fprintf(fp,
            "BEGIN:VCALENDAR\n"
            "VERSION:2.0\n"
            "PRODID:-//Sowicm.com//CSU Export Class Info as iCalendar//EN\n"
            );
    pstart = buf;
    while (true)
    {
        int i = KMP(pstart, "<td width='21%' align='left' >", true);
        if (i < 0)
            break;
        pstart += i;
        if (strncmp(pstart, "<br></td>", 9) != 0)
        {
            if (!name.empty())
                insertEvent(fp, name, teachers, weeks, when, where);
            teacher = "";
            weeks   = "";
            when    = "";
            where   = "";
            teachers.clear();
            while (*pstart)
                if (*pstart++ == ']')
                    break;
            pend = pstart;
            while (*++pend)
                if (*pend == '<')
                    break;
            name = move(string(pstart, pend));
            pstart = pend;
        }
        i = KMP(pstart, "<td width='7%' align='left' >", true);
        if (i < 0)
            break;
        pstart += i;
        if (strncmp(pstart, "<br></td>", 9) != 0)
        {
            while (*pstart)
                if (*pstart++ == '>')
                    break;
            pend = pstart;
            while (*++pend)
                if (*pend == '<')
                    break;
            bool had = false;
            teacher = move(string(pstart, pend));
            for (auto itr = teachers.begin(); itr != teachers.end(); ++itr)
            {
                if (*itr == teacher)
                {
                    had = true;
                    break;
                }
            }
            if (!had)
                teachers.push_back(move(teacher));
            pstart = pend;
        }
        i = KMP(pstart, "<td width='9%' align='left' >", true);
        if (i < 0)
            break;
        pstart += i;
        if (strncmp(pstart, "<br></td>", 9) != 0)
        {
            pend = pstart;
            while (*++pend)
                if (*pend == '<')
                    break;
            weeks = move(string(pstart, pend));
            pstart = pend;
        }
        i = KMP(pstart, "<td width='9%' align='left' >", true);
        if (i < 0)
            break;
        pstart += i;
        if (strncmp(pstart, "<br></td>", 9) != 0)
        {
            pend = pstart;
            while (*++pend)
                if (*pend == '<')
                    break;
            if (when.empty())
                when = move(string(pstart, pend));
            else if (when != string(pstart, pend))
            {
                insertEvent(fp, name, teachers, weeks, when, where);
                when = move(string(pstart, pend));
            }
            pstart = pend;
        }
        i = KMP(pstart, "<td width='13%' align='left' >", true);
        if (i < 0)
            break;
        pstart += i;
        if (strncmp(pstart, "<br></td>", 9) != 0)
        {
            pend = pstart;
            while (*++pend)
                if (*pend == '<')
                    break;
            where = move(string(pstart, pend));
            pstart = pend;
        }
    }
    if (!name.empty())
        insertEvent(fp, name, teachers, weeks, when, where);
    fprintf(fp, "END:VCALENDAR");
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData))
        exit(1);
    SHttp http;
    /*
    string s;
    printf("学号:");
    char stuid[32];
    scanf_s("%s", stuid, 31);
    printf("教务系统密码:");
    char jwcpwd[32];
    getpwd(jwcpwd);
    s = move((string)
        "typeName=%D1%A7%C9%FA"
        "&Sel_Type=STU"
        "&UserID=" + stuid +
        "&PassWord=" + jwcpwd);
    while (true)
    {
        printf("正在登录教务系统...");
        printf(s.c_str());
        http.post(
            "http://csujwc.its.csu.edu.cn/_data/index_login.aspx",
            s.c_str());
        if (KMP((const char*)http.responseContent().data,
                "正在加载权限数据...") < 0)
        {
            puts("失败!");
            printf("3秒后重试...");
            Sleep(3000);
            putchar('\r');
            continue;
        }
        puts("成功!");
        const char* buf = (char*)http.response().data;
        const char* pstart;
        const char* pend;
        int i = KMP(buf, "Set-Cookie:", 0, true);
        pend = pstart = buf + i;
        while (*++pend != '\r');
        s = move((string)
            "User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)\r\n"
            "Cookie: " + string(pstart, pend) + "\r\n"
            "Connection: Keep-Alive\r\n"
            "\r\n");
        http.setHeaderTemplate(s.c_str());
        break;
    }
    */
    printf("Cookie:");
    char cookie[1024];
    gets(cookie);
    printf("\nCookie:%s\n", cookie);
    string s;
    s = move((string)
             "User-Agent: Mozilla/5.0 (compatible; MSIE 9.0; Windows NT 6.1; WOW64; Trident/5.0)\r\n"
             "Cookie: " + cookie + "\r\n"
             "Connection: Keep-Alive\r\n"
             "\r\n");
    http.setHeaderTemplate(s.c_str());

    puts("正在获取课程表...");
    http.post("http://csujwc.its.csu.edu.cn/znpk/Pri_StuSel_rpt.aspx",
    //http.post("/znpk/Pri_StuSel_rpt.aspx",
              "Sel_XNXQ=20121&rad=1&px=0"
              );
    translate((char*)http.responseContent().data);
    puts("Done!");
    system("pause");
}
