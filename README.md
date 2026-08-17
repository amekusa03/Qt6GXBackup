# GXBackup

**GXBackup** は、Qt6/Qt5 および C++17 で構築された Linux 向けの高性能 GUI バックアップアプリケーションです。  
バックエンドに `rsync` を採用し、ハードリンクを活用した効率的なスナップショットバックアップ、システム負荷に応じた自動一時停止、定時スケジュール実行などを備えています。

![GXBackup Icon](resources/icons/gxbackup.png)

---

## 🌟 主な特徴 (Features)

- 📂 **プロファイル管理 & プリセット**
  - バックアップ元・先フォルダ、除外ルールをプロファイルごとに保持。
  - 「ユーザーデータ」「システム設定」用のワンクリック設定プリセットを搭載。

- 📸 **タイムマシン風スナップショット**
  - `rsync` の `--link-dest` オプションを活用し、変更のないファイルはハードリンクで共有。ストレージ容量を節約しつつ、過去の各時点の状態を復元可能なスナップショットとして保存。

- ⚡ **スマート負荷制限 (Smart Auto-Pause)**
  - CPU使用率や Load Average をリアルタイム監視 (`SystemMonitor`)。
  - 重い処理中や高負荷時にはバックアップを自動一時停止し、システムが空き状態に戻るとシームレスに再開。

- ⏰ **スケジュール実行 & 追いつき実行 (Catch-Up)**
  - 毎日 / 毎週の指定時刻に自動バックアップを実行。
  - PC電源がオフなどでスケジュール時刻を逃した場合も、次回起動時に自動で検知して「追いつき実行」。

- 🔔 **システムトレイ常駐 & 自動起動設定**
  - タスクトレイに常駐し、バックグラウンドで最小限のリソースで動作。
  - OS起動時の自動スタートアップ設定をサポート。

- 📊 **リアルタイムログ & 実行履歴**
  - 転送速度、進捗率、転送量、残り時間のリアルタイム表示。
  - 過去のバックアップ成否や詳細メッセージを履歴ダイアログで一元確認。

---

## 📋 動作要件 (Requirements)

### 必須コンポーネント
- **OS**: Linux (Ubuntu, Debian, Fedora, Arch Linux 等)
- **バックエンドツール**: `rsync`
- **ビルドツール**: CMake (3.16 以上), C++17 対応コンパイラ (GCC 8+ / Clang 7+)
- **フレームワーク**: Qt 5 または Qt 6 (`Core`, `Gui`, `Widgets`, `Concurrent`)

#### 依存パッケージのインストール例 (Debian/Ubuntu)
```bash
sudo apt update
sudo apt install build-essential cmake qt6-base-dev rsync
```

---

## 🛠 ビルドと実行方法 (Building & Running)

### 1. リポジトリのクローン
```bash
git clone https://github.com/amekusa03/GXBackup.git
cd GXBackup
```

### 2. ビルド
```bash
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### 3. 実行
```bash
./GXBackup
```

> **バックグラウンド起動 (最小化状態):**  
> `./GXBackup --minimized` で起動すると、起動時からシステムトレイに収納された状態で開始します。

---

## 📁 プロジェクト構成 (Project Structure)

```text
GXBackup/
├── CMakeLists.txt         # CMake ビルド設定ファイル
├── resources/             # アイコンおよび Qt リソース定義 (.qrc)
└── src/
    ├── main.cpp           # アプリケーションのエントリポイント
    ├── core/              # バックエンドロジック
    │   ├── AutostartManager.cpp / .h  # 自動起動設定 (.desktop)
    │   ├── BackupController.cpp / .h  # バックアップ処理全体統括
    │   ├── HistoryManager.cpp / .h   # バックアップ実行履歴の読み書き
    │   ├── ProfileManager.cpp / .h   # バックアッププロファイルの読み書き
    │   ├── RsyncProcess.cpp / .h     # rsync プロセスの制御・進捗解析
    │   ├── ScheduleManager.cpp / .h  # 定時スケジュール＆追いつき判定
    │   └── SystemMonitor.cpp / .h    # CPU/LoadAvg 監視
    └── ui/                # Qt GUI コンポーネント
        ├── HistoryDialog.cpp / .h    # 履歴閲覧ダイアログ
        ├── LogViewer.cpp / .h        # リアルタイムログ閲覧窓
        ├── MainWindow.cpp / .h       # メイン画面＆トレイアイコン
        └── ProfileDialog.cpp / .h    # プロファイル作成・編集画面
```

---

## 📝 使い方 (Usage Guide)

1. **プロファイルの作成**
   - 画面左上の「新規プロファイル」ボタンをクリックします。
   - バックアップ元ディレクトリと保存先ディレクトリを選択します。
   - 必要に応じてスケジュールや負荷制限しきい値を設定し、「保存」を押します。
2. **バックアップの開始**
   - リストからプロファイルを選択し、「開始」ボタンを押します。
   - プログレスバーやログで進捗状況を確認できます。
3. **履歴の確認**
   - 「履歴」ボタンを押すと、過去のバックアップ結果の一覧を確認できます。

---

## 📄 ライセンス (License)

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
