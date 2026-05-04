# Environment Setup

このページは、作業を始める前にローカルで確認する最低限の環境メモです。
詳細なツール一覧は [Investigation Tooling Guide](../tools/investigation_tooling_guide.md) を参照します。

## 最初に確認するコマンド

```sh
git status --short --branch
rtk mdbook build docs
```

`git status` では、現在のブランチ、未コミット差分、未追跡ファイルを確認します。
`rtk mdbook build docs` は docs を編集したときの最低限の検証です。

## 必須に近いツール

| ツール | 用途 |
| --- | --- |
| Git | ブランチ、差分、コミット、push の管理 |
| ripgrep | データ定義、呼び出し元、既存例の検索 |
| mdBook | `docs/` の Markdown 構成確認 |
| C compiler / make | ソース変更後のビルド確認 |

## あると便利なツール

| ツール | 用途 |
| --- | --- |
| GitHub CLI | PR 状態、CI、レビューコメントの確認 |
| mGBA | ROM の実機寄り確認 |
| JSON-aware editor | generated data や補助 JSON の確認 |

## docs-only 作業の検証

docs だけを変更した場合は、原則として次で十分です。

```sh
rtk mdbook build docs
```

現在の mdBook では、既存の `docs/CREDITS.md` にある閉じタグ由来の warning が出ることがあります。
新しく追加したページにリンク切れや HTML 解釈の warning が増えていないかを確認します。

## ソースを変更した場合の検証

ソース、include、data、tools を変更する場合は、変更内容に応じてビルドやランタイム確認を追加します。

```sh
make -j4
```

並列数は環境に合わせます。
バトル、フィールド技、マップ、セーブに関係する変更は、ビルドだけでは十分ではありません。
該当する画面やイベントを ROM 上で確認します。

## GitHub CLI 認証

PR の作成、CI 詳細、レビューコメント確認に GitHub CLI を使う場合は、認証状態を先に見ます。

```sh
gh auth status
```

認証が切れている場合は、作業者が勝手に token を作らず、ユーザーの許可を取ってからログインします。
