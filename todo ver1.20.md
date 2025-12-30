場所別ランダマイザー 設計メモ（ver1.2）
========================================

目的（新規で詰める内容）
- 野生ポケモンの時間帯別テーブル（MORNING/EVENING/NIGHTなど）を考慮した設計を詰める。
- 既存ランダマイザー＋WL/BL適用の上に、時間帯別の候補切り替えが乗る前提で、仕様/データ/バリデーションを整理する。

提案方針（YAML拡張の素案）
- 適用条件: プロジェクトの時間帯制御がONのときのみ時間帯別WLを参照。OFFなら従来の共通WLのみ使用。
- データ構造: `areas.[entry].timeSlots.{morning/evening/night/...}` を追加し、共通フィールド（apply/remove/maxRerolls/allowLegendOverrideなど）を時間帯ごとに上書きできるようにする。共通部はデフォルト、時間帯キーがあれば優先。
- 挙動: 時間帯ON→現在の時間帯のtimeSlotを優先、無ければ共通。時間帯OFF→共通のみ。
- 空WL: 時間帯ON時は空WL（出現なし）を許容。時間帯OFFで共通WLが空なら警告を出す（ビルドエラーにはしない）。
- 時間帯ON時の不足チェック: 時間帯ONにもかかわらず、どの時間帯キー（morning/evening/nightなど）も定義がない場合は設計ミスとしてビルドエラーにする。時間帯キーが全て存在するが中身が空（全時間帯で空配列）の場合は警告を出す。
  - Land/Water: timeSlotsがあれば時間帯ごとにapply/remove等を上書き、無ければ共通（通常の12/5スロット運用）。  
  - Rock: Land/Waterと同様に時間帯別WLを適用可（5スロット）。timeSlotsが無ければ共通Rock設定。  
  - Fish（釣り）: 時間帯ONなら該当時間帯の釣りエントリがあればそれを優先（ロッド別slotMode/rare/maxSpeciesも上書き可）、無ければ共通釣り設定。時間帯キーなしで時間帯ONはビルドエラー。全時間帯が空WLなら警告。時間帯OFFなら共通釣り設定のみ。
- 例（イメージ）
  ```
  route_102_land:
    key: { mapGroup: MAP_GROUP(MAP_ROUTE102), mapNum: MAP_NUM(MAP_ROUTE102), areaMask: Land }
    apply: [starter_land]
    remove: [starter_trim]
    timeSlots:
      night:
        apply: [night_only_kit]
        remove: [starter_land]
      morning:
        apply: [starter_land, morning_extra]
  ```

関連ファイル（時間帯別野生テーブル）
- `src/wild_encounter.c`  
  - `GetTimeOfDayForEncounters` で現在の時間帯を取得し、`gWildMonHeaders[headerId].encounterTypes[timeOfDay].land/water/rock/fishing/hidden` を選択。  
  - ランダマイザはこの選択後のテーブルに対して種置換/WLフィルタを行う。
- `src/data/wild_encounters.h`（生成物）  
  - `tools/wild_encounters/wild_encounters_to_header.py` で生成される野生テーブル本体。時間帯別に `encounterTypes[timeOfDay]` が展開されている。
- `src/data/wild_encounters.json`（元データ）  
  - 野生出現の元JSON。時間帯別のデータをここに記述し、生成スクリプトでCヘッダ化。
- 時間帯設定: `include/rtc.h`, `src/rtc.c`  
  - `GetTimeOfDay` / `GenConfigTimeOfDay` で時間帯を計算（世代設定による補正含む）。
- 参照箇所（閲覧/チェック用）: `dexnav.c`, `pokedex_area_screen.c`, `match_call.c` などでも `GetTimeOfDayForEncounters` を呼んでいる。
- デバッグで時間帯操作: `src/debug.c` に時間帯メニューあり（`DebugAction_TimeMenu_ChangeTimeOfDay` など）。デバッグメニューから時間帯を切り替えて挙動確認が可能。

今後詰める事項（案）
- WL/BLを時間帯別に分けるか、共通WLを時間帯選択後に適用するかの方針決定。  
- 時間帯テーブルに対してスロット再構成・レア枠を適用する場合、メタ情報をどこで持つか（YAML拡張か別テーブルか）。  
- 空WLの扱い: 時間帯ON時は空WL（出現なし）を許容する仕様とする。時間帯OFFで共通WLが空の場合は警告を出す（ビルドエラーにはしない）。  
- バリデーション: 時間帯ごとにWLが空/不足にならないか、ビルド時チェックをどうするか。  
- デバッグ: 時間帯を指定して遭遇テストできるようにするか（Var/Flagで強制時間帯を選択するフックが必要か）。

懸念点・要決定（方針を固定）
- timeSlotsの網羅性: 時間帯ONなのに一部時間帯しか定義していないのは設計ミスとしてビルドエラーにする（寛容運用はとらない）。必ず全時間帯キーを用意する。  
- 空WLの警告範囲: 一部時間帯だけ空WL（出現なし）のケースは許容（警告なし）。全時間帯空なら警告、時間帯キーなしで時間帯ONはビルドエラー。  
- マージ規則: timeSlotsに書かなかったフィールド（apply/remove/maxRerolls/allowLegendOverrideなど）は共通を継承する前提で明記する。  
- バリデーション強度: `maxSpecies < rareSlots` や slotMode矛盾など既存チェックを時間帯エントリにも同じ厳しさで適用する（継承後の最終値に対して判定）。  
- 釣りの時間帯: ロッド別設定は時間帯で上書き可。時間帯欠落時は共通ロッド設定を使用、時間帯キーなし＋時間帯ONはビルドエラー、全時間帯空は警告、として明記する。時間帯別を持たせる場合はold/good/super全ロッド必須（欠けていたらビルドエラー）。  
  - 適用順の明示: 共通値をベースに → timeSlotで書かれたフィールドだけ上書き →（釣りなら）ロッドごとに共通→timeSlotロッドで上書き → 最終値に対してバリデーション（slotMode矛盾やmaxSpecies/rareSlotsチェックなど）。

時間帯キー（YAML）とenumの対応
- enum TimeOfDay（include/constants/rtc.h）：`TIME_MORNING`, `TIME_DAY`, `TIME_EVENING`, `TIME_NIGHT`。  
- YAMLのtimeSlotsキーは `morning/day/evening/night` に固定し、この4つを全て用意する（欠けたらビルドエラー）。未知キーやスペル違いはエラー扱い。  
- 時間帯の種別（個数）はコード側のenum/TIMES_OF_DAY_COUNTに固定しておき、YAML側で増減させない（仕様を固定）。`OW_TIME_OF_DAY_FALLBACK` もグローバル設定のまま使う。
  - フォールバック先はグローバル設定（`OW_TIME_OF_DAY_FALLBACK`）のみを使用し、YAMLでの個別指定は行わない方針（仕様を固定）。将来エリア別フォールバックを入れる場合は別途拡張として扱う。

時間帯設定スイッチ（参照のみ）
- include/config/overworld.h の Time設定で挙動を制御する（実装は既存のまま利用）。  
  - `OW_TIME_OF_DAY_ENCOUNTERS`: TRUEで時間帯別エンカを有効。FALSEなら共通テーブルのみ。  
  - `OW_TIME_OF_DAY_DISABLE_FALLBACK`: TRUEで時間帯テーブルが空でもフォールバックせず出現なし。FALSEならフォールバック先を使う。  
  - `OW_TIME_OF_DAY_FALLBACK`: フォールバック先の時間帯（時間帯ONで参照先timeSlotが空だったときに切り替える先）。  
  - `OW_TIMES_OF_DAY`: 時間帯区分の計算をどの世代仕様で行うか（enum `TIME_MORNING/TIME_DAY/TIME_EVENING/TIME_NIGHT` は固定）。  
  - 空WLとフォールバックの組み合わせ: 時間帯ONでtimeSlotsに空WLが定義されており、かつ `OW_TIME_OF_DAY_DISABLE_FALLBACK=TRUE` の場合は警告のみで「その時間帯は出現なし」とする。`OW_TIME_OF_DAY_DISABLE_FALLBACK=FALSE` なら `OW_TIME_OF_DAY_FALLBACK` に従って共通/別時間帯へフォールバック（フォールバック先の時間帯テーブルを使う）。  
  - timeSlotが空でない場合: `OW_TIME_OF_DAY_DISABLE_FALLBACK` の値に関わらず、当該時間帯のWLをそのまま使用（1件でも入っていれば通常処理）。フォールバックは空の場合にのみ関与。  
  - 出現なしを明示したい場合: timeSlotを空配列にし、`OW_TIME_OF_DAY_DISABLE_FALLBACK=TRUE` をセットする（警告のみで出現なし）。デバッグログでも「時間帯ON＋空WL＋フォールバック無効」などのメッセージを出す運用にすると確認しやすい。
  - `OW_TIME_OF_DAY_DISABLE_FALLBACK` はグローバル設定で全エリアに適用される（timeSlotが空のときのフォールバック有無を全エリア共通で決める）。timeSlotに1件でもあれば通常処理。
  - 具体例: `OW_TIME_OF_DAY_DISABLE_FALLBACK=FALSE` かつ `OW_TIME_OF_DAY_FALLBACK=TIME_NIGHT`、morningが空でnightに2件あれば、morningでもnightの2件が出現する。`OW_TIME_OF_DAY_DISABLE_FALLBACK=TRUE` ならmorningは出現なし（nightは通常通り）。
  - 運用方針: 基本は `DISABLE_FALLBACK=TRUE` 推奨。`DISABLE_FALLBACK=FALSE` を使う場合は、落ち先のtimeSlotにあるレア枠設定もそのまま引き継がれる（レア枠なしの時間帯→レア枠ありの時間帯にフォールバックすることを許容）。  
  - フォールバック多段時: 落ち先も空だった場合は morning→day→evening→night→morning… の順で巡回し、最初に非空のtimeSlotを採用する。全て空なら出現なし＋警告。
  - フォールバック巡回の停止条件: 巡回はTIME_COUNT回までに打ち切り、非空が見つからなければ出現なし＋警告で確定（無限ループ防止）。
  - 意図的な出現なしを維持したい場合: timeSlotを空にしたうえで `DISABLE_FALLBACK=TRUE` をセットする（`FALSE` だとフォールバック先が使われる）。

継承ルール（timeSlotsで未記載の項目）
- 共通から継承するフィールド: apply/remove/maxRerolls/allowLegendOverride/slotMode/rareSlots/rareRate/maxSpecies/maxSpeciesOverrides。timeSlotsに書いたフィールドだけを上書きする。  
- 釣りのロッド別: 時間帯エントリにロッドが揃っている場合、各ロッドで未記載のフィールドは共通（釣りエントリ内）のロッド設定を継承。  

ログ・バリデーション実装方針
- デバッグログ: 時間帯ON＋空WL＋フォールバック無効などのケースはWARNで出力して確認しやすくする（デバッグフラグON時）。  
- 生成スクリプトのエラー/警告（時間帯関連を含む）:  
  - エラー: timeSlots欠落（時間帯ONなのに1つもなし）、未知時間帯キー、時間帯ONで全時間帯キー未定義、maxSpecies < rareSlots、slotMode矛盾（uniform＋rare指定）、rareRate>100、rareSlots=0でrareRate>0、釣り時間帯でロッド欠け、時間帯キーなし＋時間帯ON、その他既存のバリデーション違反。  
  - 警告: 時間帯ONで全時間帯空WL（出現なし）、時間帯OFFで共通WLが空、フォールバック巡回しても全時間帯空で出現なしに確定したとき。警告はビルド時に出す（Codex側でも表示可）、実機ではWARNログで確認する想定。意図的な出現なしはtimeSlotsを空配列で明示する（定義漏れではなく意図と分かるように）。  
