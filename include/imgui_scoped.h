// checkout https://github.com/ocornut/imgui/pull/2197

#pragma once
#include "imgui.h"

namespace ImGui
{
#if defined(__cplusplus) && (__cplusplus != 1 && __cplusplus != 199711L && __cplusplus != 201103L)
#define IMGUI_NODISCARD [[nodiscard]]
#elif defined(_MSC_VER) && (_MSC_VER >= 1910)
#define IMGUI_NODISCARD [[nodiscard]]
#else
#define IMGUI_NODISCARD
#endif

#define IMGUI_DELETE_MOVE_COPY(Base)                                                               \
    Base(Base&&)  = delete;               /* Move not allowed */                                   \
    Base& operator=(Base&&) = delete;     /* "" */                                                 \
    Base(const Base&)       = delete;     /* Copy not allowed */                                   \
    Base& operator=(const Base&) = delete /* "" */

    struct IMGUI_NODISCARD Window
    {
        bool IsContentVisible;

        Window(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0)
        {
            IsContentVisible = ImGui::Begin(name, p_open, flags);
        }
        ~Window()
        {
            ImGui::End();
        }

        explicit operator bool() const
        {
            return IsContentVisible;
        }

        IMGUI_DELETE_MOVE_COPY(Window);
    };

    struct IMGUI_NODISCARD Child
    {
        bool IsContentVisible;

        Child(const char* str_id, const ImVec2& size = ImVec2(0, 0), bool border = false,
              ImGuiWindowFlags flags = 0)
        {
            IsContentVisible = ImGui::BeginChild(str_id, size, border, flags);
        }
        Child(ImGuiID id, const ImVec2& size = ImVec2(0, 0), bool border = false,
              ImGuiWindowFlags flags = 0)
        {
            IsContentVisible = ImGui::BeginChild(id, size, border, flags);
        }
        ~Child()
        {
            ImGui::EndChild();
        }

        explicit operator bool() const
        {
            return IsContentVisible;
        }

        IMGUI_DELETE_MOVE_COPY(Child);
    };

    struct IMGUI_NODISCARD Font
    {
        Font(ImFont* font)
        {
            ImGui::PushFont(font);
        }
        ~Font()
        {
            ImGui::PopFont();
        }

        IMGUI_DELETE_MOVE_COPY(Font);
    };

    struct IMGUI_NODISCARD StyleColor
    {
        StyleColor(ImGuiCol idx, ImU32 col)
        {
            ImGui::PushStyleColor(idx, col);
        }
        StyleColor(ImGuiCol idx, const ImVec4& col)
        {
            ImGui::PushStyleColor(idx, col);
        }
        ~StyleColor()
        {
            ImGui::PopStyleColor();
        }

        IMGUI_DELETE_MOVE_COPY(StyleColor);
    };

    struct IMGUI_NODISCARD StyleVar
    {
        StyleVar(ImGuiStyleVar idx, float val)
        {
            ImGui::PushStyleVar(idx, val);
        }
        StyleVar(ImGuiStyleVar idx, const ImVec2& val)
        {
            ImGui::PushStyleVar(idx, val);
        }
        ~StyleVar()
        {
            ImGui::PopStyleVar();
        }

        IMGUI_DELETE_MOVE_COPY(StyleVar);
    };

    struct IMGUI_NODISCARD ItemWidth
    {
        ItemWidth(float item_width)
        {
            ImGui::PushItemWidth(item_width);
        }
        ~ItemWidth()
        {
            ImGui::PopItemWidth();
        }

        IMGUI_DELETE_MOVE_COPY(ItemWidth);
    };

    struct IMGUI_NODISCARD TextWrapPos
    {
        TextWrapPos(float wrap_pos_x = 0.0f)
        {
            ImGui::PushTextWrapPos(wrap_pos_x);
        }
        ~TextWrapPos()
        {
            ImGui::PopTextWrapPos();
        }

        IMGUI_DELETE_MOVE_COPY(TextWrapPos);
    };

    struct IMGUI_NODISCARD AllowKeyboardFocus
    {
        AllowKeyboardFocus(bool allow_keyboard_focus)
        {
            ImGui::PushAllowKeyboardFocus(allow_keyboard_focus);
        }
        ~AllowKeyboardFocus()
        {
            ImGui::PopAllowKeyboardFocus();
        }

        IMGUI_DELETE_MOVE_COPY(AllowKeyboardFocus);
    };

    struct IMGUI_NODISCARD ButtonRepeat
    {
        ButtonRepeat(bool repeat)
        {
            ImGui::PushButtonRepeat(repeat);
        }
        ~ButtonRepeat()
        {
            ImGui::PopButtonRepeat();
        }

        IMGUI_DELETE_MOVE_COPY(ButtonRepeat);
    };

    struct IMGUI_NODISCARD Group
    {
        Group()
        {
            ImGui::BeginGroup();
        }
        ~Group()
        {
            ImGui::EndGroup();
        }

        IMGUI_DELETE_MOVE_COPY(Group);
    };

    struct IMGUI_NODISCARD ID
    {
        ID(const char* str_id)
        {
            ImGui::PushID(str_id);
        }
        ID(const char* str_id_begin, const char* str_id_end)
        {
            ImGui::PushID(str_id_begin, str_id_end);
        }
        ID(const void* ptr_id)
        {
            ImGui::PushID(ptr_id);
        }
        ID(int int_id)
        {
            ImGui::PushID(int_id);
        }
        ~ID()
        {
            ImGui::PopID();
        }

        IMGUI_DELETE_MOVE_COPY(ID);
    };

    struct IMGUI_NODISCARD Combo
    {
        bool IsOpen;

        Combo(const char* label, const char* preview_value, ImGuiComboFlags flags = 0)
        {
            IsOpen = ImGui::BeginCombo(label, preview_value, flags);
        }
        ~Combo()
        {
            if (IsOpen)
                ImGui::EndCombo();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(Combo);
    };

    struct IMGUI_NODISCARD TreeNode
    {
        bool IsOpen;

        TreeNode(const char* label)
        {
            IsOpen = ImGui::TreeNode(label);
        }
        TreeNode(const char* str_id, const char* fmt, ...) IM_FMTARGS(3)
        {
            va_list ap;
            va_start(ap, fmt);
            IsOpen = ImGui::TreeNodeV(str_id, fmt, ap);
            va_end(ap);
        }
        TreeNode(const void* ptr_id, const char* fmt, ...) IM_FMTARGS(3)
        {
            va_list ap;
            va_start(ap, fmt);
            IsOpen = ImGui::TreeNodeV(ptr_id, fmt, ap);
            va_end(ap);
        }
        ~TreeNode()
        {
            if (IsOpen)
                ImGui::TreePop();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(TreeNode);
    };

    struct IMGUI_NODISCARD TreeNodeV
    {
        bool IsOpen;

        TreeNodeV(const char* str_id, const char* fmt, va_list args) IM_FMTLIST(3)
        {
            IsOpen = ImGui::TreeNodeV(str_id, fmt, args);
        }
        TreeNodeV(const void* ptr_id, const char* fmt, va_list args) IM_FMTLIST(3)
        {
            IsOpen = ImGui::TreeNodeV(ptr_id, fmt, args);
        }
        ~TreeNodeV()
        {
            if (IsOpen)
                ImGui::TreePop();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(TreeNodeV);
    };

    struct IMGUI_NODISCARD TreeNodeEx
    {
        bool IsOpen;

        TreeNodeEx(const char* label, ImGuiTreeNodeFlags flags = 0)
        {
            IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen));
            IsOpen = ImGui::TreeNodeEx(label, flags);
        }
        TreeNodeEx(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, ...) IM_FMTARGS(4)
        {
            va_list ap;
            va_start(ap, fmt);
            IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen));
            IsOpen = ImGui::TreeNodeExV(str_id, flags, fmt, ap);
            va_end(ap);
        }
        TreeNodeEx(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, ...) IM_FMTARGS(4)
        {
            va_list ap;
            va_start(ap, fmt);
            IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen));
            IsOpen = ImGui::TreeNodeExV(ptr_id, flags, fmt, ap);
            va_end(ap);
        }
        ~TreeNodeEx()
        {
            if (IsOpen)
                ImGui::TreePop();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(TreeNodeEx);
    };

    struct IMGUI_NODISCARD TreeNodeExV
    {
        bool IsOpen;

        TreeNodeExV(const char* str_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
            IM_FMTLIST(4)
        {
            IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen));
            IsOpen = ImGui::TreeNodeExV(str_id, flags, fmt, args);
        }
        TreeNodeExV(const void* ptr_id, ImGuiTreeNodeFlags flags, const char* fmt, va_list args)
            IM_FMTLIST(4)
        {
            IM_ASSERT(!(flags & ImGuiTreeNodeFlags_NoTreePushOnOpen));
            IsOpen = ImGui::TreeNodeExV(ptr_id, flags, fmt, args);
        }
        ~TreeNodeExV()
        {
            if (IsOpen)
                ImGui::TreePop();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(TreeNodeExV);
    };

    struct IMGUI_NODISCARD MainMenuBar
    {
        bool IsOpen;

        MainMenuBar()
        {
            IsOpen = ImGui::BeginMainMenuBar();
        }
        ~MainMenuBar()
        {
            if (IsOpen)
                ImGui::EndMainMenuBar();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(MainMenuBar);
    };

    struct IMGUI_NODISCARD MenuBar
    {
        bool IsOpen;

        MenuBar()
        {
            IsOpen = ImGui::BeginMenuBar();
        }
        ~MenuBar()
        {
            if (IsOpen)
                ImGui::EndMenuBar();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(MenuBar);
    };

    struct IMGUI_NODISCARD Menu
    {
        bool IsOpen;

        Menu(const char* label, bool enabled = true)
        {
            IsOpen = ImGui::BeginMenu(label, enabled);
        }
        ~Menu()
        {
            if (IsOpen)
                ImGui::EndMenu();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(Menu);
    };

    struct IMGUI_NODISCARD Tooltip
    {
        Tooltip()
        {
            ImGui::BeginTooltip();
        }
        ~Tooltip()
        {
            ImGui::EndTooltip();
        }

        IMGUI_DELETE_MOVE_COPY(Tooltip);
    };

    struct IMGUI_NODISCARD Popup
    {
        bool IsOpen;

        Popup(const char* str_id, ImGuiWindowFlags flags = 0)
        {
            IsOpen = ImGui::BeginPopup(str_id, flags);
        }
        ~Popup()
        {
            if (IsOpen)
                ImGui::EndPopup();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(Popup);
    };

    struct IMGUI_NODISCARD PopupContextItem
    {
        bool IsOpen;

        PopupContextItem(const char* str_id = NULL, int mouse_button = 1)
        {
            IsOpen = ImGui::BeginPopupContextItem(str_id, mouse_button);
        }
        ~PopupContextItem()
        {
            if (IsOpen)
                ImGui::EndPopup();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(PopupContextItem);
    };

    struct IMGUI_NODISCARD PopupContextWindow
    {
        bool IsOpen;

        PopupContextWindow(const char* str_id = NULL, int mouse_button = 1,
                           bool also_over_items = true)
        {
            IsOpen = ImGui::BeginPopupContextWindow(str_id, mouse_button, also_over_items);
        }
        ~PopupContextWindow()
        {
            if (IsOpen)
                ImGui::EndPopup();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(PopupContextWindow);
    };

    struct IMGUI_NODISCARD PopupContextVoid
    {
        bool IsOpen;

        PopupContextVoid(const char* str_id = NULL, int mouse_button = 1)
        {
            IsOpen = ImGui::BeginPopupContextVoid(str_id, mouse_button);
        }
        ~PopupContextVoid()
        {
            if (IsOpen)
                ImGui::EndPopup();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(PopupContextVoid);
    };

    struct IMGUI_NODISCARD PopupModal
    {
        bool IsOpen;

        PopupModal(const char* name, bool* p_open = NULL, ImGuiWindowFlags flags = 0)
        {
            IsOpen = ImGui::BeginPopupModal(name, p_open, flags);
        }
        ~PopupModal()
        {
            if (IsOpen)
                ImGui::EndPopup();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(PopupModal);
    };

    struct IMGUI_NODISCARD DragDropSource
    {
        bool IsOpen;

        DragDropSource(ImGuiDragDropFlags flags = 0)
        {
            IsOpen = ImGui::BeginDragDropSource(flags);
        }
        ~DragDropSource()
        {
            if (IsOpen)
                ImGui::EndDragDropSource();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(DragDropSource);
    };

    struct IMGUI_NODISCARD DragDropTarget
    {
        bool IsOpen;

        DragDropTarget()
        {
            IsOpen = ImGui::BeginDragDropTarget();
        }
        ~DragDropTarget()
        {
            if (IsOpen)
                ImGui::EndDragDropTarget();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(DragDropTarget);
    };

    struct IMGUI_NODISCARD ClipRect
    {
        ClipRect(const ImVec2& clip_rect_min, const ImVec2& clip_rect_max,
                 bool intersect_with_current_clip_rect)
        {
            ImGui::PushClipRect(clip_rect_min, clip_rect_max, intersect_with_current_clip_rect);
        }
        ~ClipRect()
        {
            ImGui::PopClipRect();
        }

        IMGUI_DELETE_MOVE_COPY(ClipRect);
    };

    struct IMGUI_NODISCARD ChildFrame
    {
        bool IsOpen;

        ChildFrame(ImGuiID id, const ImVec2& size, ImGuiWindowFlags flags = 0)
        {
            IsOpen = ImGui::BeginChildFrame(id, size, flags);
        }
        ~ChildFrame()
        {
            ImGui::EndChildFrame();
        }

        explicit operator bool() const
        {
            return IsOpen;
        }

        IMGUI_DELETE_MOVE_COPY(ChildFrame);
    };

#undef IMGUI_DELETE_MOVE_COPY

} // namespace ImGui
