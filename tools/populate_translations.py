import xml.etree.ElementTree as ET

translations_map = {
    "GXBackup - Linux Smart Backup Tool": "GXBackup - Linux スマートバックアップツール",
    "Linux Smart Backup Tool": "Linux スマートバックアップツール",
    "Start application in system tray minimized for autostart.": "自動起動のためトレイに最小化して起動します。",
    "Start application minimized to system tray.": "トレイに最小化して起動します。",
    "User Data Backup": "ユーザーデータ バックアップ",
    "Full System Backup": "システム全体 バックアップ",
    "Auto backup postponed due to high load...": "高負荷のため自動バックアップの実行を一時保留中...",
    "=== Backup Started: %1 ===": "=== バックアップ開始: %1 ===",
    "Start Time: %1": "開始日時: %1",
    "Source: %1": "ソース: %1",
    "Target: %1": "ターゲット: %1",
    "System load is high. Waiting for load to decrease before starting...": "システムが高負荷です。負荷の低下を待って開始します...",
    "[WARN] Backup start paused due to high load...": "[WARN] 高負荷のためバックアップ開始を待機中...",
    "[INFO] Manually paused": "[INFO] 手動で一時停止しました",
    "[INFO] Backup resumed": "[INFO] バックアップを再開しました",
    "[WARN] Backup canceled": "[WARN] バックアップがキャンセルされました",
    "High load detected (CPU: %1%, Load: %2) - Backup auto-paused": "高負荷を検知 (CPU: %1%, Load: %2) - バックアップを自動一時停止",
    "System load decreased - Backup resumed": "システム負荷が低下したため、バックアップを再開しました",
    "Completed successfully": "正常完了",
    "=== Backup Completed Successfully (Exit Code: %1) ===": "=== バックアップ正常完了 (Exit Code: %1) ===",
    "End Time: %1": "終了日時: %1",
    "Backup completed successfully.": "バックアップが正常に完了しました。",
    "=== Backup Failed: %1 ===": "=== バックアップ失敗: %1 ===",
    "Backup error: %1": "バックアップエラー: %1",
    "Execution Log - GXBackup": "実行ログ - GXBackup",
    "Clear Log": "ログ消去",
    "Close": "閉じる",
    "Backup Execution History - GXBackup": "バックアップ実行履歴 - GXBackup",
    "Profile Name": "プロファイル名",
    "Start Time": "開始日時",
    "End Time": "終了日時",
    "Result": "結果",
    "Details": "詳細",
    "Refresh": "更新",
    "🟢 Success": "🟢 成功",
    "🔴 Failed": "🔴 失敗",
    "Profile Settings - GXBackup": "プロファイル設定 - GXBackup",
    "Profile Name:": "プロファイル名:",
    "Backup Source:": "バックアップ元 (Source):",
    "Backup Target:": "バックアップ先 (Target):",
    "Browse...": "参照...",
    "Exclude Patterns:": "除外パターン:",
    "Exclude patterns (1 per line)\nExample:\n.cache\nDownloads\n*.tmp": "除外パターン (1行に1つ)\n例:\n.cache\nDownloads\n*.tmp",
    "📅 Automatic Schedule Execution": "📅 自動スケジュール実行",
    "Enable automatic scheduled backup": "自動スケジュールバックアップを有効にする",
    "Frequency:": "頻度:",
    "Daily": "毎日",
    "Weekly": "毎週",
    "Execution Time:": "実行時刻:",
    "Days:": "実行曜日:",
    "Mon": "月",
    "Tue": "火",
    "Wed": "水",
    "Thu": "木",
    "Fri": "金",
    "Sat": "土",
    "Sun": "日",
    "⚡ Smart Load Throttling (Auto-Pause)": "⚡ スマート負荷制限 (Auto-Pause/Throttling)",
    "Automatically pause backup on high load": "高負荷時にバックアップ処理を自動一時停止する",
    "CPU threshold for pause:": "一時停止するCPU使用率閾値:",
    "Load average threshold for pause:": "一時停止するLoad Average閾値:",
    "Save": "保存",
    "Cancel": "キャンセル",
    "Select Backup Source Directory": "バックアップ元ディレクトリを選択",
    "Select Backup Target Directory": "バックアップ先ディレクトリを選択",
    "Input Error": "入力エラー",
    "Please enter a profile name.": "プロファイル名を入力してください。",
    "Please specify backup source.": "バックアップ元を指定してください。",
    "Please specify backup target.": "バックアップ先を指定してください。",
    "&Settings": "設定(&S)",
    "&Language": "言語(&L)",
    "Backup Profiles": "バックアッププロファイル",
    "New...": "新規...",
    "Edit...": "編集...",
    "Delete": "削除",
    "📜 History": "📜 履歴",
    "Progress Status": "進捗状況",
    "Transfer Speed:": "転送速度:",
    "Transferred:": "転送量:",
    "Remaining Time:": "残り時間:",
    "System Resource Monitor (Smart Monitor)": "システム負荷状況 (Smart Monitor)",
    "CPU Usage:": "CPU使用率:",
    "Load Average (1m):": "Load Average (1分):",
    "▶ Start Backup": "▶ バックアップ開始",
    "⏸ Pause": "⏸ 一時停止",
    "▶ Resume": "▶ 再開",
    "⏹ Cancel": "⏹ キャンセル",
    "Idle": "待機中",
    "Backup in progress...": "バックアップ実行中...",
    "Paused (High load or manual)": "一時停止中 (高負荷または手動)",
    "Completed": "完了",
    "Error occurred": "エラー発生",
    "Log Output": "ログ出力",
    "📂 Open Main Window": "📂 メイン画面を開く",
    "▶ Run Backup Now": "▶ 今すぐバックアップ実行",
    "📜 View History": "📜 履歴を見る",
    "⚙ Autostart on Login": "⚙ ログイン時に自動起動",
    "🚪 Quit": "🚪 完全に終了",
    "Confirmation": "確認",
    "Backup is running. Exiting will stop the backup. Are you sure you want to quit?": "バックアップが実行中です。終了するとバックアップは停止されます。本当に完全に終了しますか？",
    "Confirm Delete": "削除確認",
    "Delete profile \"%1\"?": "プロファイル「%1」を削除しますか？",
    "🟢 Auto-pause active (CPU > %1%, Load > %2)": "🟢 自動一時停止有効 (CPU > %1%, Load > %2)",
    "⚪ Auto-pause OFF": "⚪ 自動一時停止OFF",
    "Error": "エラー",
    "Please select a profile.": "プロファイルを選択してください。",
    "Backup Error": "バックアップエラー",
    "Failed to update autostart setting.": "自動起動設定の更新に失敗しました。"
}

tree = ET.parse("translations/gxbackup_ja.ts")
root = tree.getroot()

translated_count = 0
missing = []

for context in root.findall("context"):
    for msg in context.findall("message"):
        src = msg.find("source")
        if src is not None and src.text:
            text = src.text
            trans = msg.find("translation")
            if trans is not None:
                if text in translations_map:
                    trans.text = translations_map[text]
                    if "type" in trans.attrib:
                        del trans.attrib["type"]
                    translated_count += 1
                else:
                    missing.append(text)

tree.write("translations/gxbackup_ja.ts", encoding="utf-8", xml_declaration=True)
print(f"Populated {translated_count} translations. Missing: {len(missing)}")
if missing:
    print("Missing strings:", missing)
