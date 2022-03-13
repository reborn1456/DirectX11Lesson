//============================================
//ログウィンドウクラス
//============================================
class ImGuiLogWindow {
public:
	//ログをクリア
	void Clear() { m_Buf.clear(); }

	//文字列を追加
	template<class... Args>
	void AddLog(const char* fmt, Args... args)
	{
		//改行を加える
		std::string str = fmt;
		str += "\n";
		m_Buf.appendf(str.c_str(), args...);
		m_ScrollToButtom = true;
	}

	//ウィンドウ描画
	void ImGuiUpdate(const char* title, bool* p_opende = NULL)
	{
		ImGui::Begin(title, p_opende);
		ImGui::TextUnformatted(m_Buf.begin());
		if (m_ScrollToButtom)
			ImGui::SetScrollHere(1.0f);
		m_ScrollToButtom = false;
		ImGui::End();
	}

private:
	ImGuiTextBuffer m_Buf;
	bool m_ScrollToButtom;
};