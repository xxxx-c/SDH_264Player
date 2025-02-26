#pragma once

#include <vector>
#include "MyMenu.h"
#include "MyVideoPlayer.h"

#define WM_LOOK_PICTURE_OPEN (WM_USER + 301)


typedef struct tagJIPS_COLORREF
{
    int r;
    int g;
    int b;
}JIPS_COLORREF;


//����KRectScale(...)�Ĳ����ṹ��
typedef struct tagKRectScale
{
    RECT dst; //�ͻ�������
    RECT src; //ԭʼλͼ�У���Ҫ�Ŵ�ľ���
    POINT org_left_top; //����ԭʼλͼ�Ŵ��ľ��ε����Ͻ����
}KRectScale;


enum KDraw_Mode
{
    DRAW_MODE_NONE, //û�л���
    DRAW_MODE_BEGIN, //��ʼ����
    DRAW_MODE_DRAWING, //���ڻ���
    DRAW_MODE_END //��������
};


enum KDraw_Shape
{
    DRAW_SHAPE_NONE, //û����״
    DRAW_SHAPE_POINTS, //��һ���
    DRAW_SHAPE_LINE, //��һ���߶�
    DRAW_SHAPE_LINES, //��һ����β��ӵ���
    DRAW_SHAPE_RECT, //��һ������
    DRAW_SHAPE_ELLIPSE, //��һ����Բ
    DRAW_SHAPE_POLYGON, //��һ����ն����
};


typedef struct tagDrawShapes
{
    KDraw_Shape draw_shape;
    std::vector<POINT> vector_point; //������ڻ��ߵĵ�����
    std::vector<JIPS_COLORREF> vector_color; //�߶ε���ɫ
}DrawShapes;


// CMyStatic

class CMyStatic : public CStatic
{
    DECLARE_DYNAMIC(CMyStatic)

public:
    CMyStatic();
    virtual ~CMyStatic();
    
public:
    void *m_ppvBits;
    int m_frame_num_now;
    int m_total_num_frames;

    CMyVideoPlayer m_player;

    CH264Picture m_outPicture;

    //-------------------
    int m_guid_index; //�����ͼƬ��ʽ��0-ImageFormatBMP,1-ImageFormatJPEG,2-ImageFormatPNG,3-ImageFormatGIF
    BITMAP m_bitmap;
    HBITMAP m_hbitmap;
    BITMAP m_bitmap_copy; //λͼ��һ������

    char m_temp_filename[260]; //��ʱ�ļ�

    BOOL m_bLButtonDown; //�������Ƿ���

    KRectScale m_st_rect_scale;

    float m_zoom_scale; //Ŀǰ�������ŵı���
    float m_zoom_scale_will; //�������ŵı���

    int m_mouse_move_last_x;
    int m_mouse_move_last_y;
    int m_mouse_move_delta_x; //����ƶ�ƫ����
    int m_mouse_move_delta_y; //����ƶ�ƫ����

    int m_mouse_scale_point_x; //�����ֹ���ʱ�ĵ�
    int m_mouse_scale_point_y; //�����ֹ���ʱ�ĵ�

    BOOL m_Is_Popup_Right_Menu; //�����Ƿ񵯳��Ҽ��˵�
    KDraw_Mode m_Draw_Mode; //����״̬
    KDraw_Shape m_Draw_Shape; //������״

    POINT m_pt_mouse_move; //����ƶ�ʱ�ĵ�
    std::vector<DrawShapes> m_vector_draw_shapes; //��Ŷ���ģʽ���ڻ��ߵĵ�����
    std::vector<POINT> m_vector_point; //������ڻ��ߵĵ�����
    int m_max_points_num; //���������3����

    CMyStatic * m_pAnotherClass; //���ڹ�����һ����ָ��

    RECT m_rect_render; //��Ҫ��Ⱦ�ľ�������(Ĭ�ϲ���Ⱦ)
    BOOL m_IsAcceptDropFiles; //�Ƿ������ҷ�ļ�(Ĭ����)

    CString m_file_path_open; //����򿪵��ļ�����

    JIPS_COLORREF m_color_background; //������ͼ�ı�����ɫ(Ĭ�Ϻ�ɫ)
    JIPS_COLORREF m_color_draw_shape; //�������ߵ���ɫ(Ĭ�Ϻ�ɫ)
    
    CMyMenu m_menu;
    int m_currMbAddr;

public:
    int SetHwndParent(HWND hwnd_parent);

    int start_play(const char *url);
    int stop_play();
    int pause_play(bool isPause);

    static int H264VideoPlayerFrameCallback(CH264Picture *outPicture, void *userData, int errorCode);

    int mb_hit_test(POINT pt, CH264Picture *Pic, CH264MacroBlock &mb); //�����в���

    //----------------------------
    int image_set_render_rect(int left, int top, int right, int bottom); //������Ⱦ����(������Ȥ����)
    int image_set_render_rect_by_ROI(); //������Ⱦ����(������Ȥ����)
    int SetAnotherClass(CMyStatic * p);
    int GetAnotherClass(CMyStatic * p);
    int OpenImage(char* filename, BITMAP &bitmap, HBITMAP &hBitmap); //��ͼƬ
    int SaveImage(char* filename, HBITMAP &hBitmap); //����ͼƬ,���λͼ���
    int SaveImage(char* filename, BITMAP &bitmap); //����ͼƬ������ڴ���λͼ���
    int CreateEmptyImage(BITMAP &bitmap, int width, int height, int bmBitsPixel); //���ڴ��д���һ���հ�λͼ
    int ReleaseHandle(); //�����ͷ���Դ
    int ReleaseBitmap(BITMAP &bitmap); //�����ͷ���Դ
    int Convert_Bitmap_BitsPixel_8_to_24(BITMAP &bitmap_24, BITMAP &bitmap_8); //��8λλͼת����24λ
    int Cut_Image(BITMAP &bitmap_src, BITMAP &bitmap_dst, int x, int y, int w, int h); //�ü�λͼ��һ����������

    //---------ֱ�����ڴ�DC�в���-------------------
    int Draw_Rect_DC(HDC hdc, int x, int y, int w, int h, int r, int g, int b, int line_width/*=1*/); //��λͼ��ָ��λ�û�ָ����ɫ�ľ��ο�
    int Draw_Ellipse_DC(HDC hdc, int x, int y, int w, int h, int r, int g, int b, int line_width/*=1*/); //��λͼ��ָ��λ�û�ָ����ɫ����Բ
    int Draw_Line_DC(HDC hdc, POINT pt1, POINT pt2, int r, int g, int b, int line_width/*=1*/); //��ֱ��
    int Draw_Polygon_DC(HDC hdc, std::vector<POINT> &vector_point, int r, int g, int b, int line_width/*=1*/); //�������
    int Draw_Triangle_DC(HDC hdc, POINT pt1, POINT pt2, POINT pt3, int r, int g, int b, int line_width/*=1*/); //��λͼ��ָ��λ�û�ָ����ɫ��������
    int Draw_Points_DC(HDC hdc, std::vector<POINT> &vector_point, int r, int g, int b, int line_width/*=1*/); //����

    int picture_open(CString filename);
    int picture_open_bitmap(BITMAP bitmap);
    int picture_open_bitmap_fresh(BITMAP &bitmap); //ר������ͼƬ����ˢ��
    int picture_close();
    int picture_copy(BITMAP bitmap_src, BITMAP &bitmap_dst); //����һ��λͼ
    int picture_paint(HDC hdc, int dst_x, int dst_y, int dst_width, int dst_height, BITMAP &bitmap); //��Ƶ��ͼ
    virtual int picture_draw_callback(HDC hdc); //���̳������
    int picture_draw_shape(HDC hdc); //������ͼ��

public:
    int CalcIntersectRect(); //������ν���
    int CenterVideo(); //����Ƶ�����������Ļ����(��Ҫʱ���ŵ���Ļ���ڴ�С)
    int ConvertPt2Pt(POINT in, POINT &out); //�����ڿͻ�������ת����ԭʼͼƬ����
    int SetDrawShapeParam(KDraw_Shape draw_shape, std::vector<POINT> vector_point); //���û��ߵĲ���
    int GetDrawShapeParam(KDraw_Shape &draw_shape, std::vector<POINT> &vector_point); //��ȡ���ߵĲ���
    int GetBitampFromDC(BITMAP &Bitmap, BOOL isDrawShape = FALSE);
    int GetMergeBitmap(BITMAP &Bitmap, BOOL isDrawShape = FALSE); //�൱�ںϲ�ͼ��
    int Set_Is_Accept_Drop_Files(BOOL isAcceptDropFiles);
    int Set_Background_Color(int r, int g, int b); //���ñ�����ɫ
    int Set_Draw_Shape_Line_Color(int r, int g, int b); //���û��ߵ���ɫ
    int Set_Draw_Shape_Mode(KDraw_Shape draw_shape); //���û�����״

    DECLARE_MESSAGE_MAP()

    afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
    virtual void PreSubclassWindow();
    afx_msg void OnPaint();
    afx_msg BOOL OnEraseBkgnd(CDC* pDC);
    afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
    afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
    virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
    afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
    afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
    afx_msg void OnMouseMove(UINT nFlags, CPoint point);
    afx_msg void OnSize(UINT nType, int cx, int cy);
    afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
    afx_msg void OnDropFiles(HDROP hDropInfo);
    afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg UINT OnGetDlgCode();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
    afx_msg void OnTimer(UINT_PTR nIDEvent);
};


