#include "WebViewDialog.hpp"

#include "I18N.hpp"
#include "slic3r/GUI/wxExtensions.hpp"
#include "slic3r/GUI/GUI_App.hpp"
#include "slic3r/GUI/MainFrame.hpp"
#include "libslic3r_version.h"
#include "../Utils/Http.hpp"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

#include <wx/sizer.h>
#include <wx/toolbar.h>
#include <wx/textdlg.h>
#include <wx/url.h>

#include <slic3r/GUI/Widgets/WebView.hpp>
#include "print_manage/AppMgr.hpp"
#include "libslic3r/common_header/common_header.h"
namespace pt = boost::property_tree;

namespace Slic3r { namespace GUI {
wxDECLARE_EVENT(EVT_RESPONSE_MESSAGE, wxCommandEvent);

wxDEFINE_EVENT(EVT_RESPONSE_MESSAGE, wxCommandEvent);

#define LOGIN_INFO_UPDATE_TIMER_ID 10002

BEGIN_EVENT_TABLE(WebViewPanel, wxPanel)
EVT_TIMER(LOGIN_INFO_UPDATE_TIMER_ID, WebViewPanel::OnFreshLoginStatus)
END_EVENT_TABLE()

WebViewPanel::WebViewPanel(wxWindow* parent) : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize)
{
#if CUSTOM_COMMUNITY_ENABLE
    wxString url = GetURL();
    wxBoxSizer* topsizer = new wxBoxSizer(wxVERTICAL);

#if !BBL_RELEASE_TO_PUBLIC
    // Create the button
    bSizer_toolbar = new wxBoxSizer(wxHORIZONTAL);

    m_button_back = new wxButton(this, wxID_ANY, wxT("Back"), wxDefaultPosition, wxDefaultSize, 0);
    m_button_back->Enable(false);
    bSizer_toolbar->Add(m_button_back, 0, wxALL, 5);

    m_button_forward = new wxButton(this, wxID_ANY, wxT("Forward"), wxDefaultPosition, wxDefaultSize, 0);
    m_button_forward->Enable(false);
    bSizer_toolbar->Add(m_button_forward, 0, wxALL, 5);

    m_button_stop = new wxButton(this, wxID_ANY, wxT("Stop"), wxDefaultPosition, wxDefaultSize, 0);

    bSizer_toolbar->Add(m_button_stop, 0, wxALL, 5);

    m_button_reload = new wxButton(this, wxID_ANY, wxT("Reload"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer_toolbar->Add(m_button_reload, 0, wxALL, 5);

    m_url = new wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
    bSizer_toolbar->Add(m_url, 1, wxALL | wxEXPAND, 5);

    m_button_tools = new wxButton(this, wxID_ANY, wxT("Tools"), wxDefaultPosition, wxDefaultSize, 0);
    bSizer_toolbar->Add(m_button_tools, 0, wxALL, 5);

    topsizer->Add(bSizer_toolbar, 0, wxEXPAND, 0);
    bSizer_toolbar->Show(false);

    // Create panel for find toolbar.
    wxPanel* panel = new wxPanel(this);
    topsizer->Add(panel, wxSizerFlags().Expand());

    // Create sizer for panel.
    wxBoxSizer* panel_sizer = new wxBoxSizer(wxVERTICAL);
    panel->SetSizer(panel_sizer);
#endif // BBL_RELEASE_TO_PUBLIC
    // Create the info panel
    m_info = new wxInfoBar(this);
    topsizer->Add(m_info, wxSizerFlags().Expand());
    // Create the webview
    m_browser = WebView::CreateWebView(this, url);
    m_browser->EnableAccessToDevTools(); // for debug:
    if (m_browser == nullptr) {
        wxLogError("Could not init m_browser");
        return;
    }
    // m_browser->Hide();
    SetSizer(topsizer);

    topsizer->Add(m_browser, wxSizerFlags().Expand().Proportion(1));

    // Log backend information
    /* m_browser->GetUserAgent() may lead crash
    if (wxGetApp().get_mode() == comDevelop) {
        wxLogMessage(wxWebView::GetBackendVersionInfo().ToString());
        wxLogMessage("Backend: %s Version: %s", m_browser->GetClassInfo()->GetClassName(),
            wxWebView::GetBackendVersionInfo().ToString());
        wxLogMessage("User Agent: %s", m_browser->GetUserAgent());
    }
    */

    // Create the Tools menu
    m_tools_menu           = new wxMenu();
    wxMenuItem* viewSource = m_tools_menu->Append(wxID_ANY, _L("View Source"));
    wxMenuItem* viewText   = m_tools_menu->Append(wxID_ANY, _L("View Text"));
    m_tools_menu->AppendSeparator();
    m_tools_handle_navigation = m_tools_menu->AppendCheckItem(wxID_ANY, _L("Handle Navigation"));
    m_tools_handle_new_window = m_tools_menu->AppendCheckItem(wxID_ANY, _L("Handle New Windows"));
    m_tools_menu->AppendSeparator();

    // Create an editing menu
    wxMenu* editmenu = new wxMenu();
    m_edit_cut       = editmenu->Append(wxID_ANY, _L("Cut"));
    m_edit_copy      = editmenu->Append(wxID_ANY, _L("Copy"));
    m_edit_paste     = editmenu->Append(wxID_ANY, _L("Paste"));
    editmenu->AppendSeparator();
    m_edit_undo = editmenu->Append(wxID_ANY, _L("Undo"));
    m_edit_redo = editmenu->Append(wxID_ANY, _L("Redo"));
    editmenu->AppendSeparator();
    m_edit_mode = editmenu->AppendCheckItem(wxID_ANY, _L("Edit Mode"));
    m_tools_menu->AppendSubMenu(editmenu, "Edit");

    wxMenu* script_menu = new wxMenu;
    m_script_string     = script_menu->Append(wxID_ANY, "Return String");
    m_script_integer    = script_menu->Append(wxID_ANY, "Return integer");
    m_script_double     = script_menu->Append(wxID_ANY, "Return double");
    m_script_bool       = script_menu->Append(wxID_ANY, "Return bool");
    m_script_object     = script_menu->Append(wxID_ANY, "Return JSON object");
    m_script_array      = script_menu->Append(wxID_ANY, "Return array");
    m_script_dom        = script_menu->Append(wxID_ANY, "Modify DOM");
    m_script_undefined  = script_menu->Append(wxID_ANY, "Return undefined");
    m_script_null       = script_menu->Append(wxID_ANY, "Return null");
    m_script_date       = script_menu->Append(wxID_ANY, "Return Date");
    m_script_message    = script_menu->Append(wxID_ANY, "Send script message");
    m_script_custom     = script_menu->Append(wxID_ANY, "Custom script");
    m_tools_menu->AppendSubMenu(script_menu, _L("Run Script"));
    wxMenuItem* addUserScript      = m_tools_menu->Append(wxID_ANY, _L("Add user script"));
    wxMenuItem* setCustomUserAgent = m_tools_menu->Append(wxID_ANY, _L("Set custom user agent"));

    // Selection menu
    wxMenu* selection     = new wxMenu();
    m_selection_clear     = selection->Append(wxID_ANY, _L("Clear Selection"));
    m_selection_delete    = selection->Append(wxID_ANY, _L("Delete Selection"));
    wxMenuItem* selectall = selection->Append(wxID_ANY, _L("Select All"));

    editmenu->AppendSubMenu(selection, "Selection");

    wxMenuItem* loadscheme  = m_tools_menu->Append(wxID_ANY, _L("Custom Scheme Example"));
    wxMenuItem* usememoryfs = m_tools_menu->Append(wxID_ANY, _L("Memory File System Example"));

    m_context_menu = m_tools_menu->AppendCheckItem(wxID_ANY, _L("Enable Context Menu"));
    m_dev_tools    = m_tools_menu->AppendCheckItem(wxID_ANY, _L("Enable Dev Tools"));

    // By default we want to handle navigation and new windows
    m_tools_handle_navigation->Check();
    m_tools_handle_new_window->Check();

    // Zoom
    m_zoomFactor = 100;

    // Connect the button events
#if !BBL_RELEASE_TO_PUBLIC
    Bind(wxEVT_BUTTON, &WebViewPanel::OnBack, this, m_button_back->GetId());
    Bind(wxEVT_BUTTON, &WebViewPanel::OnForward, this, m_button_forward->GetId());
    Bind(wxEVT_BUTTON, &WebViewPanel::OnStop, this, m_button_stop->GetId());
    Bind(wxEVT_BUTTON, &WebViewPanel::OnReload, this, m_button_reload->GetId());
    Bind(wxEVT_BUTTON, &WebViewPanel::OnToolsClicked, this, m_button_tools->GetId());
    Bind(wxEVT_TEXT_ENTER, &WebViewPanel::OnUrl, this, m_url->GetId());

#endif // BBL_RELEASE_TO_PUBLIC

    // Connect the webview events
    Bind(wxEVT_WEBVIEW_NAVIGATING, &WebViewPanel::OnNavigationRequest, this);
    Bind(wxEVT_WEBVIEW_NAVIGATED, &WebViewPanel::OnNavigationComplete, this);
    Bind(wxEVT_WEBVIEW_LOADED, &WebViewPanel::OnDocumentLoaded, this);
    Bind(wxEVT_WEBVIEW_TITLE_CHANGED, &WebViewPanel::OnTitleChanged, this);
    Bind(wxEVT_WEBVIEW_ERROR, &WebViewPanel::OnError, this);
    Bind(wxEVT_WEBVIEW_NEWWINDOW, &WebViewPanel::OnNewWindow, this);
    Bind(wxEVT_WEBVIEW_SCRIPT_MESSAGE_RECEIVED, &WebViewPanel::OnScriptMessage, this);
    Bind(EVT_RESPONSE_MESSAGE, &WebViewPanel::OnScriptResponseMessage, this);

    // Connect the menu events
    Bind(wxEVT_MENU, &WebViewPanel::OnViewSourceRequest, this, viewSource->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnViewTextRequest, this, viewText->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnCut, this, m_edit_cut->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnCopy, this, m_edit_copy->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnPaste, this, m_edit_paste->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnUndo, this, m_edit_undo->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRedo, this, m_edit_redo->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnMode, this, m_edit_mode->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptString, this, m_script_string->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptInteger, this, m_script_integer->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptDouble, this, m_script_double->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptBool, this, m_script_bool->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptObject, this, m_script_object->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptArray, this, m_script_array->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptDOM, this, m_script_dom->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptUndefined, this, m_script_undefined->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptNull, this, m_script_null->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptDate, this, m_script_date->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptMessage, this, m_script_message->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnRunScriptCustom, this, m_script_custom->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnAddUserScript, this, addUserScript->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnSetCustomUserAgent, this, setCustomUserAgent->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnClearSelection, this, m_selection_clear->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnDeleteSelection, this, m_selection_delete->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnSelectAll, this, selectall->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnLoadScheme, this, loadscheme->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnUseMemoryFS, this, usememoryfs->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnEnableContextMenu, this, m_context_menu->GetId());
    Bind(wxEVT_MENU, &WebViewPanel::OnEnableDevTools, this, m_dev_tools->GetId());

    // Connect the idle events
    Bind(wxEVT_IDLE, &WebViewPanel::OnIdle, this);
    Bind(wxEVT_CLOSE_WINDOW, &WebViewPanel::OnClose, this);

    m_LoginUpdateTimer = nullptr;

#ifdef __WXGTK__
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    SetDoubleBuffered(true);
    m_freshTimer = new wxTimer();
    m_freshTimer->SetOwner(this);
    m_freshTimer->Start(500);
    Bind(wxEVT_TIMER, [this](wxTimerEvent&){
        this->Refresh();
    });
#endif
    DM::AppMgr::Ins().Register(m_browser);
#endif
}

wxString WebViewPanel::GetURL()
{
    // alpha/beta/dev 对社区的影响：社区使用不同的创想云服务环境
    // alpha：预发布环境（pre）+ cpp 启动的本地 http 服务
    // beta：生产环境 + c++ 启动的本地 http 服务
    // dev：根据开发需求决定用哪个环境 + 前端开发启动的局域网 http 服务

    // alpha & beta: c++ 启动了一个 http server
    // 原因：file 协议加载 html 跨域问题难以处理，尤其是大文件（font 文件，10M+）
    // http server 信息：
    // 1. 运行在 http://127.0.0.1:%d（端口动态获取）
    // 2. 将 resources_dir 的 web/homepage 作为静态目录，社区的 html 等打包放在这里
    // 3. c++ webview 加载 http://127.0.0.1:%d/#/；添加 # 是因为使用了 hash 路由
    // wxString url     = wxString::Format("http://127.0.0.1:%d/#/",wxGetApp().get_server_port()); // 127.0.0.1 访问创想云 cdn 图片会
    // 403，因此使用 localhost
    wxString lang = wxGetApp().app_config->get("language") != "" ? wxGetApp().app_config->get("language") : "en_GB";

    std::string type = std::string(PROJECT_VERSION_EXTRA);
    type.erase(std::remove(type.begin(), type.end(), ' '), type.end());

    std::string version = std::string(CREALITYPRINT_VERSION);
    version.erase(std::remove(version.begin(), version.end(), ' '), version.end());

    std::string region = wxGetApp().app_config->get("region");

    std::string use_inches = wxGetApp().app_config->get("use_inches");

    int port = wxGetApp().get_server_port();
    //type = std::string("Beta");
    // for pro
    #ifdef _DEBUG1
    type = std::string("Beta");
    #endif
    wxString url = wxString::Format(
        "http://localhost:%d/homepage/index.html?lang=%s&version=%s&type=%s&region=%s&use_inches=%s&debug=false&time=%d#/Community/Home Page", port,
        lang, version, type, region, use_inches,std::time(0));

    // for dev: 使用局域网地址：可以找【刘明，或其他前端开发同事】，按要求启动一个 http 服务，用于调试
     /*port = 9090;
     type = std::string("Beta");
     
     wxString url = wxString::Format(
         "http://localhost:%d/index.html?lang=%s&version=%s&type=%s&region=%s&use_inches=%s&debug=false&time=%d#/Community/Home Page", port, lang,
         version, type, region, use_inches,use_inches,std::time(0)
     );*/
     return url;
}
WebViewPanel::~WebViewPanel()
{
#ifdef __WXGTK__
    m_freshTimer->Stop();
    m_browser->Stop();
    m_browser->RemoveScriptMessageHandler("wx");
#endif
#if CUSTOM_COMMUNITY_ENABLE
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << " Start";
    SetEvtHandlerEnabled(false);
    DM::AppMgr::Ins().UnRegister(m_browser);
    delete m_tools_menu;

    if (m_LoginUpdateTimer != nullptr) {
        m_LoginUpdateTimer->Stop();
        delete m_LoginUpdateTimer;
        m_LoginUpdateTimer = NULL;
    }
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << " End";
#endif
}
void WebViewPanel::Reload()
{
    m_browser->LoadURL(GetURL());
    //m_browser->SetFocus();
    UpdateState();
}
void WebViewPanel::load_url(wxString& url)
{
#if CUSTOM_COMMUNITY_ENABLE
    this->Show();
    this->Raise();
    m_url->SetLabelText(url);

    if (wxGetApp().get_mode() == comDevelop)
        wxLogMessage(m_url->GetValue());
    m_browser->LoadURL(url);
    m_browser->SetFocus();
    UpdateState();
#endif
}

/**
 * Method that retrieves the current state from the web control and updates the GUI
 * the reflect this current state.
 */
void WebViewPanel::UpdateState()
{
#if !BBL_RELEASE_TO_PUBLIC
    if (m_browser->CanGoBack()) {
        m_button_back->Enable(true);
    } else {
        m_button_back->Enable(false);
    }
    if (m_browser->CanGoForward()) {
        m_button_forward->Enable(true);
    } else {
        m_button_forward->Enable(false);
    }
    if (m_browser->IsBusy()) {
        m_button_stop->Enable(true);
    } else {
        m_button_stop->Enable(false);
    }

    // SetTitle(m_browser->GetCurrentTitle());
    m_url->SetValue(m_browser->GetCurrentURL());
#endif // BBL_RELEASE_TO_PUBLIC
}

void WebViewPanel::OnIdle(wxIdleEvent& WXUNUSED(evt))
{
#if !BBL_RELEASE_TO_PUBLIC
    if (m_browser->IsBusy()) {
        wxSetCursor(wxCURSOR_ARROWWAIT);
        m_button_stop->Enable(true);
    } else {
        wxSetCursor(wxNullCursor);
        m_button_stop->Enable(false);
    }
#endif // BBL_RELEASE_TO_PUBLIC
}

/**
 * Callback invoked when user entered an URL and pressed enter
 */
void WebViewPanel::OnUrl(wxCommandEvent& WXUNUSED(evt))
{
    if (wxGetApp().get_mode() == comDevelop)
        wxLogMessage(m_url->GetValue());
    m_browser->LoadURL(m_url->GetValue());
    m_browser->SetFocus();
    UpdateState();
}

/**
 * Callback invoked when user pressed the "back" button
 */
void WebViewPanel::OnBack(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->GoBack();
    UpdateState();
}

/**
 * Callback invoked when user pressed the "forward" button
 */
void WebViewPanel::OnForward(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->GoForward();
    UpdateState();
}

/**
 * Callback invoked when user pressed the "stop" button
 */
void WebViewPanel::OnStop(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->Stop();
    UpdateState();
}

/**
 * Callback invoked when user pressed the "reload" button
 */
void WebViewPanel::OnReload(wxCommandEvent& WXUNUSED(evt))
{
    m_browser->Reload();
    UpdateState();
}

void WebViewPanel::OnCut(wxCommandEvent& WXUNUSED(evt)) { m_browser->Cut(); }

void WebViewPanel::OnCopy(wxCommandEvent& WXUNUSED(evt)) { m_browser->Copy(); }

void WebViewPanel::OnPaste(wxCommandEvent& WXUNUSED(evt)) { m_browser->Paste(); }

void WebViewPanel::OnUndo(wxCommandEvent& WXUNUSED(evt)) { m_browser->Undo(); }

void WebViewPanel::OnRedo(wxCommandEvent& WXUNUSED(evt)) { m_browser->Redo(); }

void WebViewPanel::OnMode(wxCommandEvent& WXUNUSED(evt)) { m_browser->SetEditable(m_edit_mode->IsChecked()); }

void WebViewPanel::OnLoadScheme(wxCommandEvent& WXUNUSED(evt))
{
    wxPathList pathlist;
    pathlist.Add(".");
    pathlist.Add("..");
    pathlist.Add("../help");
    pathlist.Add("../../../samples/help");

    wxFileName helpfile(pathlist.FindValidPath("doc.zip"));
    helpfile.MakeAbsolute();
    wxString path = helpfile.GetFullPath();
    // Under MSW we need to flip the slashes
    path.Replace("\\", "/");
    path = "wxfs:///" + path + ";protocol=zip/doc.htm";
    m_browser->LoadURL(path);
}

void WebViewPanel::OnUseMemoryFS(wxCommandEvent& WXUNUSED(evt)) { m_browser->LoadURL("memory:page1.htm"); }

void WebViewPanel::OnEnableContextMenu(wxCommandEvent& evt) { m_browser->EnableContextMenu(evt.IsChecked()); }

void WebViewPanel::OnEnableDevTools(wxCommandEvent& evt) { m_browser->EnableAccessToDevTools(evt.IsChecked()); }

void WebViewPanel::OnClose(wxCloseEvent& evt) { this->Hide(); }

void WebViewPanel::OnFreshLoginStatus(wxTimerEvent& event)
{
    auto mainframe = Slic3r::GUI::wxGetApp().mainframe;
    if (mainframe && mainframe->m_webview == this)
        Slic3r::GUI::wxGetApp().get_login_info();
}

void WebViewPanel::SetLoginPanelVisibility(bool bshow)
{
    wxString strJS = wxString::Format("SetLoginPanelVisibility(%s)", bshow ? "true" : "false");
    RunScript(strJS);
}
void WebViewPanel::SendRecentList(int images)
{
    boost::property_tree::wptree req;
    boost::property_tree::wptree data;
    wxGetApp().mainframe->get_recent_projects(data, images);
    req.put(L"sequence_id", "");
    req.put(L"command", L"get_recent_projects");
    req.put_child(L"response", data);
    std::wostringstream oss;
    pt::write_json(oss, req, false);
    RunScript(wxString::Format("window.handleStudioCmd(%s)", oss.str()));
}

void WebViewPanel::ChangeRegion(int regionCode)
{
    boost::property_tree::wptree req;
    boost::property_tree::wptree data;

    req.put(L"sequence_id", "");
    req.put(L"command", L"get_current_region");
    req.put(L"region_code", regionCode);
    req.put_child(L"response", data);
    std::wostringstream oss;
    pt::write_json(oss, req, false);
    RunScript(wxString::Format("window.handleStudioCmd(%s)", oss.str()));
}

void WebViewPanel::sync_preferences(const std::vector<wxString>& prefs)
{
    if (prefs.size() < 4)
        return;

    boost::property_tree::wptree req;
    req.put(L"command", L"sync_preference");

    boost::property_tree::wptree preferences;
    preferences.put(L"lang", prefs[0].ToStdWstring());
    preferences.put(L"region", prefs[1].ToStdWstring());
    preferences.put(L"use_inches", prefs[2].ToStdWstring());
    preferences.put(L"syncPresetEnabled", prefs[3].ToStdWstring());
    req.add_child(L"preference", preferences);

    std::wostringstream oss;
    pt::write_json(oss, req, false);

    RunScript(wxString::Format("window.handleStudioCmd(%s)", oss.str()));
}

void WebViewPanel::update_device_page(const std::string& device_info)
{
    RunScript(wxString::Format("window.handleStudioCmd('%s')", from_u8(device_info)));
}
// tao
void WebViewPanel::update_all_device(const std::string& jsStr) { RunScript(from_u8(jsStr)); }

void WebViewPanel::SendDesignStaffpick(bool on)
{
    // if (on) {
    //     get_design_staffpick(0, 60, [this](std::string body) {
    //         if (body.empty() || body.front() != '{') {
    //             BOOST_LOG_TRIVIAL(warning) << "get_design_staffpick failed " + body;
    //             return;
    //         }
    //         CallAfter([this, body] {
    //             auto body2 = from_u8(body);
    //             body2.insert(1, "\"command\": \"modelmall_model_advise_get\", ");
    //             RunScript(wxString::Format("window.handleStudioCmd(%s)", body2));
    //         });
    //     });
    // } else {
    //     std::string body2 = "{\"total\":0, \"hits\":[]}";
    //     body2.insert(1, "\"command\": \"modelmall_model_advise_get\", ");
    //     RunScript(wxString::Format("window.handleStudioCmd(%s)", body2));
    // }
}

void WebViewPanel::OpenModelDetail(std::string id, NetworkAgent* agent)
{
    std::string url;
    if ((agent ? agent->get_model_mall_detail_url(&url, id) : get_model_mall_detail_url(&url, id)) == 0) {
        if (url.find("?") != std::string::npos) {
            url += "&from=crealityprint";
        } else {
            url += "?from=crealityprint";
        }

        wxLaunchDefaultBrowser(url);
    }
}

void WebViewPanel::SendLoginInfo()
{
    if (wxGetApp().getAgent()) {
        std::string login_info = wxGetApp().getAgent()->build_login_info();
        wxString    strJS      = wxString::Format("window.handleStudioCmd(%s)", login_info);
        RunScript(strJS);
    }
}

void WebViewPanel::ShowNetpluginTip()
{
    // Install Network Plugin
    // std::string NP_Installed = wxGetApp().app_config->get("installed_networking");
    bool bValid = wxGetApp().is_compatibility_version();

    int nShow = 0;
    if (!bValid)
        nShow = 1;

    BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << boost::format(": bValid=%1%, nShow=%2%") % bValid % nShow;

    json m_Res           = json::object();
    m_Res["command"]     = "network_plugin_installtip";
    m_Res["sequence_id"] = "10001";
    m_Res["show"]        = nShow;

    wxString strJS = wxString::Format("window.handleStudioCmd(%s)", m_Res.dump(-1, ' ', false, json::error_handler_t::ignore));

    RunScript(strJS);
}

void WebViewPanel::get_design_staffpick(int offset, int limit, std::function<void(std::string)> callback)
{
    // auto host = wxGetApp().get_http_url(wxGetApp().app_config->get_country_code(), "v1/design-service/design/staffpick");
    // std::string url = (boost::format("%1%/?offset=%2%&limit=%3%") % host % offset % limit).str();

    // Http http = Http::get(url);
    // http.header("accept", "application/json")
    //     .header("Content-Type", "application/json")
    //     .on_complete([this, callback](std::string body, unsigned status) { callback(body); })
    //     .on_error([this, callback](std::string body, std::string error, unsigned status) {
    //         callback(body);
    //     })
    //     .perform();
}

int WebViewPanel::get_model_mall_detail_url(std::string* url, std::string id)
{
    // https://makerhub-qa.bambu-lab.com/en/models/2077
    std::string h = wxGetApp().get_model_http_url(wxGetApp().app_config->get_country_code());
    auto        l = wxGetApp().current_language_code_safe();
    if (auto n = l.find('_'); n != std::string::npos)
        l = l.substr(0, n);
    *url = (boost::format("%1%%2%/models/%3%") % h % l % id).str();
    return 0;
}

void WebViewPanel::update_mode()
{
#if CUSTOM_COMMUNITY_ENABLE
    GetSizer()->Show(size_t(0), wxGetApp().app_config->get("internal_developer_mode") == "true");
    GetSizer()->Layout();
#endif
}

/**
 * Callback invoked when there is a request to load a new page (for instance
 * when the user clicks a link)
 */
void WebViewPanel::OnNavigationRequest(wxWebViewEvent& evt)
{
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << ": " << evt.GetTarget().ToUTF8().data();
    const wxString& url = evt.GetURL();
    if (url.StartsWith("File://") || url.StartsWith("file://")) {
        if (!url.Contains("/web/homepage/index.html")) {
            auto file = wxURL::Unescape(wxURL(url).GetPath());
#ifdef _WIN32
            if (file.StartsWith('/'))
                file = file.Mid(1);
#endif
            wxGetApp().plater()->load_files(wxArrayString{1, &file});
            evt.Veto();
            return;
        }
    }

    if (m_info->IsShown()) {
        m_info->Dismiss();
    }

    if (wxGetApp().get_mode() == comDevelop)
        wxLogMessage("%s", "Navigation request to '" + evt.GetURL() + "' (target='" + evt.GetTarget() + "')");

    // If we don't want to handle navigation then veto the event and navigation
    // will not take place, we also need to stop the loading animation
    if (!m_tools_handle_navigation->IsChecked()) {
        evt.Veto();
        m_button_stop->Enable(false);
    } else {
        UpdateState();
    }
}

/**
 * Callback invoked when a navigation request was accepted
 */
void WebViewPanel::OnNavigationComplete(wxWebViewEvent& evt)
{
    m_browser->Show();
    Layout();
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << ": " << evt.GetTarget().ToUTF8().data();
    if (wxGetApp().get_mode() == comDevelop)
        wxLogMessage("%s", "Navigation complete; url='" + evt.GetURL() + "'");
    UpdateState();
    ShowNetpluginTip();
}

/**
 * Callback invoked when a page is finished loading
 */
void WebViewPanel::OnDocumentLoaded(wxWebViewEvent& evt)
{
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << ": " << evt.GetTarget().ToUTF8().data();
    // Only notify if the document is the main frame, not a subframe
    if (evt.GetURL() == m_browser->GetCurrentURL()) {
        if (wxGetApp().get_mode() == comDevelop)
            wxLogMessage("%s", "Document loaded; url='" + evt.GetURL() + "'");
    }
    UpdateState();
}

void WebViewPanel::OnTitleChanged(wxWebViewEvent& evt)
{
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << ": " << evt.GetString().ToUTF8().data();
    // wxGetApp().CallAfter([this] { SendRecentList(); });
}

/**
 * On new window, we veto to stop extra windows appearing
 */
void WebViewPanel::OnNewWindow(wxWebViewEvent& evt)
{
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << ": " << evt.GetURL().ToUTF8().data();
    wxString flag = " (other)";

    if (evt.GetNavigationAction() == wxWEBVIEW_NAV_ACTION_USER) {
        flag = " (user)";
    }

    if (wxGetApp().get_mode() == comDevelop)
        wxLogMessage("%s", "New window; url='" + evt.GetURL() + "'" + flag);

    // If we handle new window events then just load them in this window as we
    // are a single window browser
    if (m_tools_handle_new_window->IsChecked())
        m_browser->LoadURL(evt.GetURL());

    UpdateState();
}

void WebViewPanel::OnScriptMessage(wxWebViewEvent& evt)
{
    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << ": " << evt.GetString().ToUTF8().data();
    // update login status
    if (m_LoginUpdateTimer == nullptr) {
        if (Slic3r::GUI::wxGetApp().mainframe->m_webview) {
            std::vector<wxString> prefs;

            wxString    strlang           = wxGetApp().current_language_code_safe();
            std::string country_code      = wxGetApp().app_config->get("region");
            std::string use_inches        = wxGetApp().app_config->get("use_inches");
            std::string syncPresetEnabled = wxGetApp().app_config->get("sync_user_preset") == "true" ? "1" : "0";

            prefs.push_back(strlang);
            prefs.push_back(wxString(country_code));
            prefs.push_back(wxString(use_inches));
            prefs.push_back(wxString(syncPresetEnabled));
            Slic3r::GUI::wxGetApp().mainframe->m_webview->sync_preferences(prefs);
        }
        BOOST_LOG_TRIVIAL(info) << __FUNCTION__ << " Create Timer";
        m_LoginUpdateTimer = new wxTimer(this, LOGIN_INFO_UPDATE_TIMER_ID);
        m_LoginUpdateTimer->Start(2000);
    }

    if (wxGetApp().get_mode() == comDevelop)
        wxLogMessage("Script message received; value = %s, handler = %s", evt.GetString(), evt.GetMessageHandler());

    if (DM::AppMgr::Ins().Invoke(m_browser, evt.GetString().ToUTF8().data())) {
        return;
    }

    std::string response = wxGetApp().handle_web_request(evt.GetString().ToUTF8().data());
    if (response.empty())
        return;

    /* remove \n in response string */
    response.erase(std::remove(response.begin(), response.end(), '\n'), response.end());
    if (!response.empty()) {
        // m_response_js = wxString::Format("window.handleStudioCmd('%s')", response);
        wxString        wxResponse = from_u8(response.c_str());
        wxCommandEvent* event      = new wxCommandEvent(EVT_RESPONSE_MESSAGE, this->GetId());
        event->SetClientData(new wxString(wxString::Format("window.handleStudioCmd('%s')", wxResponse)));
        wxQueueEvent(this, event);
    } else {
        m_response_js.clear();
    }
}

void WebViewPanel::OnScriptResponseMessage(wxCommandEvent& evt)
{
    wxString* wxResponse = static_cast<wxString*>(evt.GetClientData());
    if (wxResponse) {
        if (!wxResponse->empty()) {
            RunScript(*wxResponse);
        }
        delete wxResponse;
    }
}

/**
 * Invoked when user selects the "View Source" menu item
 */
void WebViewPanel::OnViewSourceRequest(wxCommandEvent& WXUNUSED(evt))
{
    SourceViewDialog dlg(this, m_browser->GetPageSource());
    dlg.ShowModal();
}

/**
 * Invoked when user selects the "View Text" menu item
 */
void WebViewPanel::OnViewTextRequest(wxCommandEvent& WXUNUSED(evt))
{
    wxDialog textViewDialog(this, wxID_ANY, "Page Text", wxDefaultPosition, wxSize(700, 500), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);

    wxTextCtrl* text = new wxTextCtrl(this, wxID_ANY, m_browser->GetPageText(), wxDefaultPosition, wxDefaultSize,
                                      wxTE_MULTILINE | wxTE_RICH | wxTE_READONLY);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(text, 1, wxEXPAND);
    SetSizer(sizer);
    textViewDialog.ShowModal();
}

/**
 * Invoked when user selects the "Menu" item
 */
void WebViewPanel::OnToolsClicked(wxCommandEvent& WXUNUSED(evt))
{
    if (m_browser->GetCurrentURL() == "")
        return;

    m_edit_cut->Enable(m_browser->CanCut());
    m_edit_copy->Enable(m_browser->CanCopy());
    m_edit_paste->Enable(m_browser->CanPaste());

    m_edit_undo->Enable(m_browser->CanUndo());
    m_edit_redo->Enable(m_browser->CanRedo());

    m_selection_clear->Enable(m_browser->HasSelection());
    m_selection_delete->Enable(m_browser->HasSelection());

    m_context_menu->Check(m_browser->IsContextMenuEnabled());
    m_dev_tools->Check(m_browser->IsAccessToDevToolsEnabled());

    wxPoint position = ScreenToClient(wxGetMousePosition());
    PopupMenu(m_tools_menu, position.x, position.y);
}

void WebViewPanel::RunScript(const wxString& javascript)
{
    // Remember the script we run in any case, so the next time the user opens
    // the "Run Script" dialog box, it is shown there for convenient updating.
    m_javascript = javascript;

    if (!m_browser)
        return;

    WebView::RunScript(m_browser, javascript);
}

void WebViewPanel::OnRunScriptString(wxCommandEvent& WXUNUSED(evt)) { RunScript("setCount(345);"); }

void WebViewPanel::OnRunScriptInteger(wxCommandEvent& WXUNUSED(evt)) { RunScript("function f(a){return a;}f(123);"); }

void WebViewPanel::OnRunScriptDouble(wxCommandEvent& WXUNUSED(evt)) { RunScript("function f(a){return a;}f(2.34);"); }

void WebViewPanel::OnRunScriptBool(wxCommandEvent& WXUNUSED(evt)) { RunScript("function f(a){return a;}f(false);"); }

void WebViewPanel::OnRunScriptObject(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(){var person = new Object();person.name = 'Foo'; \
    person.lastName = 'Bar';return person;}f();");
}

void WebViewPanel::OnRunScriptArray(wxCommandEvent& WXUNUSED(evt)) { RunScript("function f(){ return [\"foo\", \"bar\"]; }f();"); }

void WebViewPanel::OnRunScriptDOM(wxCommandEvent& WXUNUSED(evt)) { RunScript("document.write(\"Hello World!\");"); }

void WebViewPanel::OnRunScriptUndefined(wxCommandEvent& WXUNUSED(evt)) { RunScript("function f(){var person = new Object();}f();"); }

void WebViewPanel::OnRunScriptNull(wxCommandEvent& WXUNUSED(evt)) { RunScript("function f(){return null;}f();"); }

void WebViewPanel::OnRunScriptDate(wxCommandEvent& WXUNUSED(evt))
{
    RunScript("function f(){var d = new Date('10/08/2017 21:30:40'); \
    var tzoffset = d.getTimezoneOffset() * 60000; \
    return new Date(d.getTime() - tzoffset);}f();");
}

void WebViewPanel::OnRunScriptMessage(wxCommandEvent& WXUNUSED(evt)) { RunScript("window.wx.postMessage('This is a web message');"); }

void WebViewPanel::OnRunScriptCustom(wxCommandEvent& WXUNUSED(evt))
{
    wxTextEntryDialog dialog(this, "Please enter JavaScript code to execute", wxGetTextFromUserPromptStr, m_javascript,
                             wxOK | wxCANCEL | wxCENTRE | wxTE_MULTILINE);
    if (dialog.ShowModal() != wxID_OK)
        return;

    RunScript(dialog.GetValue());
}

void WebViewPanel::OnAddUserScript(wxCommandEvent& WXUNUSED(evt))
{
    wxString userScript = "window.wx_test_var = 'wxWidgets webview sample';";
    wxTextEntryDialog
        dialog(this, "Enter the JavaScript code to run as the initialization script that runs before any script in the HTML document.",
               wxGetTextFromUserPromptStr, userScript, wxOK | wxCANCEL | wxCENTRE | wxTE_MULTILINE);
    if (dialog.ShowModal() != wxID_OK)
        return;

    if (!m_browser->AddUserScript(dialog.GetValue()))
        wxLogError("Could not add user script");
}

void WebViewPanel::OnSetCustomUserAgent(wxCommandEvent& WXUNUSED(evt))
{
    wxString          customUserAgent = "Mozilla/5.0 (iPhone; CPU iPhone OS 13_1_3 like Mac OS X) AppleWebKit/605.1.15 (KHTML, like Gecko) "
                                        "Version/13.0.1 Mobile/15E148 Safari/604.1";
    wxTextEntryDialog dialog(this, "Enter the custom user agent string you would like to use.", wxGetTextFromUserPromptStr, customUserAgent,
                             wxOK | wxCANCEL | wxCENTRE);
    if (dialog.ShowModal() != wxID_OK)
        return;

    if (!m_browser->SetUserAgent(customUserAgent))
        wxLogError("Could not set custom user agent");
}

void WebViewPanel::OnClearSelection(wxCommandEvent& WXUNUSED(evt)) { m_browser->ClearSelection(); }

void WebViewPanel::OnDeleteSelection(wxCommandEvent& WXUNUSED(evt)) { m_browser->DeleteSelection(); }

void WebViewPanel::OnSelectAll(wxCommandEvent& WXUNUSED(evt)) { m_browser->SelectAll(); }

/**
 * Callback invoked when a loading error occurs
 */
void WebViewPanel::OnError(wxWebViewEvent& evt)
{
#define WX_ERROR_CASE(type) \
    case type: category = #type; break;

    wxString category;
    switch (evt.GetInt()) {
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_CONNECTION);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_CERTIFICATE);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_AUTH);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_SECURITY);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_NOT_FOUND);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_REQUEST);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_USER_CANCELLED);
        WX_ERROR_CASE(wxWEBVIEW_NAV_ERR_OTHER);
    }

    BOOST_LOG_TRIVIAL(trace) << __FUNCTION__ << ": [" << category << "] " << evt.GetString().ToUTF8().data();

    if (wxGetApp().get_mode() == comDevelop) {
        wxLogMessage("%s", "Error; url='" + evt.GetURL() + "', error='" + category + " (" + evt.GetString() + ")'");

        // Show the info bar with an error
        m_info->ShowMessage(_L("An error occurred loading ") + evt.GetURL() + "\n" + "'" + category + "'", wxICON_ERROR);
    }

    UpdateState();
}

SourceViewDialog::SourceViewDialog(wxWindow* parent, wxString source)
    : wxDialog(parent, wxID_ANY, "Source Code", wxDefaultPosition, wxSize(700, 500), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)
{
    wxTextCtrl* text = new wxTextCtrl(this, wxID_ANY, source, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_RICH | wxTE_READONLY);

    wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
    sizer->Add(text, 1, wxEXPAND);
    SetSizer(sizer);
}

}} // namespace Slic3r::GUI
