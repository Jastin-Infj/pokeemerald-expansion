場所別ランダマイザー 設計メモ（ver1.1）

目的
- 野生ランダマイザーの挙動を再設計し、出現率・種数・レア枠を制御できるようにする。
- 現行は「元スロットの出現率をそのままに種族IDだけ置換」なので、均等化やレア枠追加、最大種数管理を取り入れる。

現状整理（野生）
- スロット数・出現率は元データそのまま。WLで種だけ置換。
- WLフラグON＋ルールあり: maxRerolls回リロール→WL内なら採用、上限超過でWLからランダム1種。
- WLが少なくてもフォールバックで成立。WLが多くても上限を設けず、分布はスロット/リロール次第で偏る。
- 生成スクリプトは空WLエラーのみ。出現率調整や種数制限は未対応。

やりたいこと（野生再設計案）
- スロット再構成: エリアWLから最大N種（例:12）を選び、スロット数に応じて均等割り or レア枠込みで再配置する。
- 出現率制御: 全スロット同確率にする or レア枠の比率を持たせる。比率/レア枠情報はYAMLで指定し、ビルド時にヘッダ生成する案。
- 種数のバリデーション: WL種数が要求より少ない/多い場合はビルド時にエラー or 自動で絞り込み（ランダム/優先度付き）。
- フォールバック/設計ミスの扱い: `minDistinct > wlCount` や「スロット要求数 > WL種数」は生成スクリプトで検知して落とす方針を検討。

重複制御（トレーナー）※現行仕様維持のまま
- デフォルト: maxSame=255, minDistinct=0（制限なし）。テーブルに載せたトレーナーのみ制御有効。
- WL優先で、ユニークが足りない場合は重複許容で妥協。`maxSame=0` は避ける（実質NG）。
- ビルド時チェック案: `minDistinct > wlCount` はエラー（未実装）。

デバッグ/検証（野生）
- フラグ: FLAG_RANDOMIZER_AREA_WL が必須。ログ確認は DEBUGビルド＋FLAG_RANDOMIZER_DEBUG_LOG＋適切なLOG_HANDLER。
- 目視/ログで: レア枠・均等化が意図どおりの比率になっているか、フォールバックが出ていないかを確認。

出現率モデルの切替案
- デフォルトは均等（uniform）で、エリア単位で例外的にレア枠を使いたい場合はYAMLに `slotMode` を持たせる（例: `uniform`/`rare`）。レア枠の比率は `rareSlots` や `rareRate` などでエリアごとに記述。
- エリアキー（mapGroup/mapNum）でモードを指定する。指定がなければ均等扱い。
- 将来的にランタイムVar/Flagで全体上書きも許容する設計にする（例: `VAR_WILD_SLOT_MODE`: 0=均等、1=レア枠使用）。データ側はエリアごとのデフォルト値を持ち、Varがあればそれを優先。

現行スロット仕様（既存コードの固定数）
- Landは12スロット固定で確率分岐している（ChooseWildMonIndex_Landはスロット0〜11まで）。
- Water/Rockは5スロット固定。釣りは竿ごとに2/5/5スロットで確率分岐が固定。
- 現状のコードでは12枠以上は扱っていない。増設するには分岐やテーブル参照を組み直す改修が必要。

キット分離とガチャ的出現確率の方針
- キットを野生用・トレーナー用で分離する（序盤キット/中盤キットなどを用途別に持つ）。
- 野生向けのキットから最大12種を選定する仕組みを想定し、各種に個別の出現確率を持たせる（ガチャ的な重み付け）。レア枠はその一部として扱う。
- 「登場可否の確率」と「登場後の遭遇率」を分けて設計する。例: まず一定確率で候補リストに入る（登場可否）、入った後でスロット内の重みで遭遇しにくい種を作る。
- トレーナー用は別キットを用意し、野生とは独立した重み・選定を行う。
- テンプレート化: 序盤キット等の汎用テンプレに加え、特定エリア用のキット（エリア専用構成）も持てるようにする。キットを組み合わせてapply/removeできる運用を想定。
- レア枠の扱い: レア枠と一般枠は別々に持つ（レア枠が一般枠に埋もれないようにする）。レア枠を設定する場合、必ずレア枠が存在するテーブル構成とし、出現率は変数/フラグで上書き可能とする。デフォルトは一般枠と同様に均等扱い（レア枠を設定しても遭遇率は均等化する仕様を基本とする）。
- レア枠の抽選順序: まずレア枠の当たり判定を行い、当たれば先にレア枠を確保し残りを一般枠で埋める。当たらなければ一般枠のみで構成する。全体一括抽選でレア枠が埋もれないようにする。
決める必要がある事項
- 出現率モデル: 全均等 / レア枠付き（比率指定） / 元スロットを再利用 のどれにするか。
- 最大種数: 例として12種に揃えるのか、エリアごとに指定するのか。
- バリデーション強度: ビルドエラーで止めるか、警告で通すか、または自動絞り込みにするか。

実装方針例（ソース修正はまだしない）
1) YAML拡張: レア枠比率・最大種数などのメタを追加。ビルドスクリプトでスロット構成を生成できるようにする。
2) 生成物: `generated/randomizer_area_rules.h` に加え、スロット再構成済みテーブルを出力するか、ランタイムで再構成するかを決める。
3) ランタイム: 再構成済みテーブルがあればそれを採用。なければ現行の「スロット出現率維持＋種置換」を使うフォールバックを残す。

詳細設計（野生スロット再構成の案）
- 入力: area_rules.yml に以下メタを追加する想定
  - `maxSpecies`: エリア内で使用する最大種数（例: 12）。省略時は従来どおり無制限。
  - `rareSlots`: レア枠の比率またはスロット数（例: 1/12）。省略時は均等化のみ。
  - `fallbackMode`: WL不足時の扱い（error/warn/allow）。省略時はerror推奨。
- 生成スクリプトでの処理（ビルド時）
  1) WLを読み込み、必要なら `maxSpecies` までに絞り込み（ランダム/優先度付きはいずれかを選択）。
  2) スロット数（元の出現スロット数）を取得し、均等割りで配置。`rareSlots` が指定されている場合は指定スロットをレア枠として比率を低めに設定。
  3) WL種数 < 必要種数（minDistinct相当やmaxSpeciesなど）ならエラーまたは警告。`fallbackMode` が error ならビルド失敗。
  4) 生成物として「スロット→種族ID」の再構成テーブルを出力（例: `gRandomizerAreaSlotTable[]`）。
- ランタイムでの利用（構想）
  - `RandomizeWildEncounter` でスロットから種を引く際、再構成済みテーブルがあればそれを使う。なければ従来の「RandomizeMon＋WLフィルタ」へフォールバック。
  - レア枠の比率はテーブル内のスロット配置に反映済みとし、ランタイムでは単純にスロット選択→置換のみにする。
- バリデーション
  - YAML時点で `WLが0` はエラー（現行どおり）。
  - `maxSpecies` が指定されている場合、`WL種数 < maxSpecies` ならエラーか警告（方針に合わせる）。
  - `rareSlots` が `maxSpecies` を超える、もしくはスロット数を超える場合はエラー。

決定事項（maxSpeciesとスロット数の扱い）
- スロット数よりWL種数が少ない場合は、WL定義数で打ち切る（例: スロット12、WL4種なら4種だけを使い残りはスロットから外す想定）。余剰スロットは割り当てない方向で進める。
- maxSpecies未指定時のデフォルト: 12枠とする（現行データの最大スロット数を上限とみなす）。DevNav等のUI制約もあり、12を上限に据える。
- Land以外（Water/Rock/Fishing）はスロット数が少ないため、当面は各種別の現行最大スロット数を上限とする（12枠への拡張は見送り）。例: Water/Rockは5枠、釣りは竿ごとに2/5/5枠を上限として扱う。
- 種別ごとのデフォルト上限（maxSpecies省略時）
  - Land: 12
  - Water/Rock: 5
  - Fishing: 竿ごとに 2 / 5 / 5

maxSpeciesの例外指定（案）
- 基本は種別デフォルト（上記）を全エリアに適用し、例外がある場合のみ「エリア×種別」でオーバーライドを書く。例: `maxSpeciesOverrides: { Water: 3 }` でそのエリアの水枠上限を3に変更。
- `Water: 0` のように0を指定した場合は、その種別を出現なし（スロット割り当て無し）として扱う想定。

再現性・シード方針
- ベースシードはトレーナーID＋秘密ID（現行仕様）を維持。野生はmapGroup/mapNum/area/slot、トレーナーはtrainerId/partySize/slotを組み合わせて安定化。
- スロット再構成は可能ならビルド時に確定したテーブルを使い、ランタイム再構成は避けると再現性が高い。
- ビルド時にランダム抽出する場合はスクリプト内で固定シードを使うか、優先度/順序を明示してランダムを使わない形にする。

バリデーション方針（maxSpeciesとrareSlots）
- `maxSpecies < rareSlots` の場合はビルドエラーとする（レア枠が最大種数を超える矛盾を許容しない）。

データ入力フォーマット案（例付き）
- エリアキー: `key.mapGroup/mapNum/areaMask`（必須）。
- モード: `slotMode: uniform|rare`（省略時uniform）。
- 最大種数: `maxSpecies`（省略時は種別デフォルト: Land=12, Water/Rock=5, Fishing=2/5/5）。例外は `maxSpeciesOverrides: { Water: 3, Rock: 0 }`（0でその種別なし）。
- レア枠: `rareSlots` もしくは `rareRate`。`maxSpecies < rareSlots` はビルドエラー。レア枠抽選→当たれば先に確保、外れたら一般枠のみ。
- fallbackMode: `error|warn|allow`（省略時error推奨）。
- キット適用: `apply/remove`（野生用/トレーナー用でキットを分離して運用）。
- 例1（デフォルト均等）:
  ```
  key: { mapGroup: 3, mapNum: 2, areaMask: Land }
  slotMode: uniform
  maxSpecies: 12  # 省略時Landのデフォルト
  apply: [starter_land]
  ```
- 例2（Waterだけ上限を絞り、レア枠なし）:
  ```
  slotMode: uniform
  maxSpecies: 12
  maxSpeciesOverrides: { Water: 3 }
  apply: [starter_land, mid_land]
  remove: [starter_trim]
  ```
- 例3（レア枠あり、Rockを出現なし）:
  ```
  key: { mapGroup: 3, mapNum: 2, areaMask: Land }
  slotMode: rare
  maxSpecies: 12
  maxSpeciesOverrides: { Rock: 0 }
  rareSlots: 1     # レア枠1スロット相当
  apply: [starter_land, late_land, legend_unlock]
  allowLegendOverride: true
  ```

最終案のフィールド一覧（YAML想定）
- トップレベル（エリア単位）
  - key: { mapGroup, mapNum, areaMask }  ※必須。釣りも含め共通キー（未記載はエラー）。
  - slotMode: uniform | rare
  - maxSpecies: 種別デフォルト上限を上書きする値（省略可）
  - maxSpeciesOverrides: { Water: n, Rock: n, Fishing: n, old: n, good: n, super: n, ... } 0でその種別を禁止
  - apply: [kitA, kitB, ...]
  - remove: [kitX, ...]
- areaMaskの固定リスト: Land / Water / Rock / Fishing（必須いずれか）。釣りの竿別指定は Fishingエントリ内のfishing.old/good/superで行う。
- レア指定（共通）
  - rareSlots: 数（整数、上限は種別スロット数）
  - rareRate: 0〜100（整数）。未指定時デフォルト0。rareSlotsとセットで使う。
- 釣り（オプションB: 竿別フィールドをエリア内に持つ形）
  - fishing:
      - old:  { slotMode, rareSlots, rareRate, maxSpecies }
      - good: { slotMode, rareSlots, rareRate, maxSpecies }
      - super:{ slotMode, rareSlots, rareRate, maxSpecies }
- 固定キー宣言:
  - key { mapGroup, mapNum, areaMask }（areaMaskは Land/Water/Rock/Fishing のみ。必須。未記載エラー）
  - slotMode, maxSpecies, maxSpeciesOverrides
  - apply, remove
  - rareSlots, rareRate
  - fishing.old, fishing.good, fishing.super

例（ランドの均等・rareなし）
```
key: { mapGroup: 3, mapNum: 2, areaMask: Land }
slotMode: uniform
apply: [starter_land]
```

例（水だけ上限を3に絞る）
```
key: { mapGroup: 3, mapNum: 2, areaMask: Land }
slotMode: uniform
maxSpecies: 12
maxSpeciesOverrides: { Water: 3 }
apply: [starter_land, mid_land]
remove: [starter_trim]
```

例（レア枠あり: Landで1枠レア）
```
key: { mapGroup: 3, mapNum: 2, areaMask: Land }
slotMode: rare
maxSpecies: 12
rareSlots: 1
rareRate: 100   # 必ず1枠レア
apply: [starter_land, late_land]
```

例（釣り竿別指定）
```
key: { mapGroup: 3, mapNum: 2, areaMask: Fishing }
slotMode: uniform  # 釣り共通
fishing:
  old:  { slotMode: rare, rareSlots: 1, rareRate: 20, maxSpecies: 2 }
  good: { slotMode: rare, rareSlots: 1, rareRate: 50, maxSpecies: 5 }
  super:{ slotMode: rare, rareSlots: 1, rareRate: 70, maxSpecies: 5 }
```

Fishingの竿別指定の扱い（要決定）
- オプションA: 竿ごとに別エントリを持つ（例: area+OldRod / area+GoodRod / area+SuperRodを別キーとして記述）。柔軟だがエントリが増える。
- オプションB: ひとつのエリアエントリ内で `fishing: { old: {...}, good: {...}, super: {...} }` のように竿別フィールドを持つ。構造は複雑だがエントリ数は抑えられる。
- どちらを採用しても、maxSpecies上限（Old=2, Good=5, Super=5）を超えた指定はビルドエラーにする。`0`指定でその竿種を禁止する扱いはどちらの形式でも可能。
- 運用上の簡潔さを優先するならA（エントリ分割）、データ集中管理を優先するならB（フィールド内分割）を選ぶ。
- 当面の方針: オプションBを採用。Landより釣りの出現頻度は低いが、レア枠調整しやすい利点を優先し、エリア1つに釣り情報をまとめて管理する。
- 前提補足: 釣り竿による遭遇はゲーム上の行動頻度自体が低めと想定しており、その分レア枠で個性を出しやすい設計を狙う。
- 竿別と共通rare指定の優先: 併記はエラーで止める（竿別優先で共通無視という許容案は採用しない）。
  - 決定経緯: 併記を許すと「どちらを採るか」で運用がぶれ、デバッグ負荷が増えるため。釣りは竿別で個性を出す方針に寄せ、共通rareとの併記は明示的に禁止してシンプルにする。
- 竿別デフォルトの考え方: Old < Good < Super の順でレア率を高めに設定する運用が攻略性に寄与。明示記述がなければ竿別も均等（レア枠なし）とし、レア枠を使う場合は竿別にrareSlots/rareRateを必ず書く。
- 補足: 釣りは竿別で個別にレア枠・レア率を持つ設計のまま進める（Landの単一レア枠と比べて特別扱いになるが、当面このバランスで運用）。
- fishing内のslotMode: 各竿(old/good/super)で個別に指定。rareならその竿でレア枠を使う、uniformならレア枠なし均等。
- 種別別のレア指定方針: 管理コストとバリデーション簡素化のため、Land/Water/RockなどはareaMaskごとに別エントリを書くA案で進める（1エリア内に種別別フィールドを持たせるB案は採用しない）。レア枠が必要な種別だけエントリを用意し、不要なら書かないかmaxSpecies=0で禁止。

追加方針・補足
- 竿別指定: Fishingは竿ごとにスロット上限が異なるため、必要なら竿別に指定できるようにする（変数/フラグで切替を検討しつつ、難しければ竿別記述で対応）。
- apply/removeの重複: キット合成時の重複は自動除去する（重複を重みとして扱わない）。出現率の偏りは個別重み設定のミスとして扱う。
- apply/removeの優先順: applyで足し込んだあとにremoveで差し引く（remove優先）。重複は自動除去し、順序に依存した重み付けは行わない。
- apply/remove運用の詳細:
  - 順序固定: すべてのapplyを適用してから全removeを適用する（途中で交互にしない）。
  - 重複処理: merge後に1回だけ重複除去する運用を想定。
  - スコープ分離: 野生用とトレーナー用で別スコープのapply/removeを持ち、混在させない。
  - ログ: 生成スクリプトで最終的な種数や適用キットをログ出力すると運用ミスを検知しやすい。
- 優先順位の明記（maxSpecies / apply/remove）
  - maxSpecies: 種別デフォルト（Land12/Water5/Rock5/Fish2/5/5）→ エリアのmaxSpecies → maxSpeciesOverrides（竿別含む）の順。上限超過はエラー。
  - apply/remove: applyで加算→mergeで重複除去→removeで差し引き（remove優先）。野生/トレーナーでスコープ分離。
  - maxSpecies未指定時: 種別デフォルトを採用（Landなら12など）。共通のデフォルトを基点にし、個別エリアや竿別のoverrideで上書き可能。
- バリデーションの運用: 厳格にエラーを出す方針（WL不足やレア枠矛盾など）。エラーメッセージは原因がわかる形で（例: "Water: maxSpecies < rareSlots" 等）出すことを想定。
- バリデーションの粒度案:
  - エラー対象（ビルド停止）: maxSpeciesが種別上限超え、maxSpecies < rareSlots、WL=0、0指定＋rareSlots併記、未サポートのslotMode指定。
  - 警告対象（ビルドは通すが通知）: maxSpeciesを種別上限より大きく書いた場合のクランプ（※現方針はエラー優先）、rareRateとrareSlotsの同時指定（どちらを優先するか要決定）、maxSpeciesを指定しないままrareSlotsだけ指定した場合の解釈揺れ。
  - 要決定: rareSlotsとrareRateを両方書いたときの優先順位（片方をエラーにするか、rareSlots優先などに固定するか）。
  - slotMode=uniform なのに rareSlots/rareRate を書いた場合は矛盾でエラー。
  - rareSlots=0 かつ rareRate>0 は矛盾としてエラー（レア枠がないのに確率だけ指定されているため）。
  - rare枠を使わない場合: rareSlotsを0または未記載、rareRateも未記載（0相当）にする。rareRateだけ書いて0指定、あるいはrareSlots不在でrate>0はエラー。
  - rareRateだけ指定／rareRate>100: 枠なし／上限超過としてエラー（rareRateは0〜100の整数に限定）。
  - maxSpecies未指定＋rareSlots指定: 種別デフォルトのmaxSpeciesで解釈してビルドを通す（例: Landなら12枠）。警告を出すかは運用で選択。
  - エリア0/種別0指定時: rareSlots/rareRateが残っていたら矛盾としてエラー（出現なし指定とレア枠指定は両立しない）。
  - WL不足: 必要種数（maxSpeciesやminDistinct相当）を満たさないWLは設計ミスとしてビルドエラーで止め、どのキーで不足したかを明記する。
- rareRateとrareSlotsの関係（抽選イメージ）
  - rareRateは「レア枠を何枠採用できるかの乱数上限」を決める重みで、実際の確保数はrareSlotsで上限をかける（例: 乱数で5枠当たりでもrareSlots=3なら3枠まで確保）。
  - 逆にrareRateが低く当たり数が0ならレア枠なしで一般枠のみ。
  - これを実装する場合は、エリアごとに rareSlots を確定値、rareRate を抽選用の比率として保持し、抽選結果を rareSlots でクリップする運用とする。
  - rareSlots=0 かつ rareRate>0 は矛盾としてビルドエラー（レア枠ゼロで確率だけ指定されているため）。
  - rateが高く複数枠当たりでも、確保数はrareSlotsでクリップする（例: 当たり5枠、rareSlots=3→3枠まで）。
  - slotMode=uniform のときはrareSlots/rareRateを併記しない（併記は矛盾としてエラー）。
 - rare設定の優先順位・扱い
  - slotMode/rareSlots/rareRateはエリアエントリ内で一意に書く（複数書きはエラー）。
  - 釣りは竿別フィールドがあれば竿別指定を優先（エリア共通のrare指定とは併記しないか、併記したら竿別を優先と明記する）。
  - rareSlotsのみ指定してrareRate未指定なら「rareRateはデフォルト（例: 必要に応じて1.0扱い）」などのデフォルトを決める。逆にrareRateのみ指定でrareSlots未指定はエラー（枠がないため）。
  - rare指定が全く無ければslotMode=uniformとして扱い、レア枠なし。
  - rareRateのスケール: 0〜100の百分率（整数）を想定し、100を超えたらエラー。rareRate未指定時はデフォルト0（レア枠抽選を行わない）とする。
  - まとめ: slotModeでレアを使うか決め、使う場合はrareSlotsで枠上限、rareRateで当たりやすさ（0〜100%）を決める。uniformならrare指定は併記しない。
  - 実装ガイド（コメント例）: rareSlots=1、rareRate=100で「必ず1枠レア」を作る／rareRateを下げれば発生率が下がる、といったHowToを生成コメントに残すと運用しやすい。
  - ビルドエラー条件（明示）:
    - maxSpeciesが種別上限超、またはmaxSpecies < rareSlots
  - WL=0、あるいは必要種数を満たさないWL（不足キーを明記）
  - slotMode=uniformなのにrareSlots/rareRate記載
  - rareSlots=0でrareRate>0
  - rareRate>100（0〜100以外）
    - rareSlotsとrareRateを併記（エラー推奨。許すならrareSlots優先＋警告に変更する運用も検討可）
    - 竿別/共通rareの併記（エラー推奨。許すなら竿別優先＋警告）
    - エリア0/種別0指定時にrareSlots/rareRateが残っている
  - データ型の想定: rareSlots/rareRateは整数（u8想定）。rareRateは0〜100の百分率のみを許容し、小数や重み値は扱わない。
  - rareデフォルトの方針: 種別問わずデフォルトは「レアなし」（rareRate=0、rareSlots未指定）。レア枠を使いたい場合は種別・竿別に明示記述してもらう方針でシンプルにする。
  - デフォルトを持たせる理由: 記述漏れ・設定忘れでも安全側（均等/レアなし/種別上限どおり）で動かすため。種別ごとにスロット上限が異なるので共通値ではなく種別デフォルトを採用し、必要なエリアだけoverrideを書く省力化を狙う。
- エラーメッセージの粒度例:
  - "Land: maxSpecies 13 exceeds limit 12"
  - "Water: rareSlots 6 exceeds slot limit 5"
  - "Fishing/old: rareRate 150 out of range (0-100)"
  - "Land: maxSpecies 4 < rareSlots 5"
  - "Land: WL missing or insufficient (requires >=N)"
- シード/ログの扱い:
  - スロット再構成後は新しいスロット番号（0〜maxSpecies-1）でログを出す。元の12枠などの番号は表示しない。
  - レア枠抽選の結果（当たり枠数と最終確保枠数）をログ出力すると、rareRate/rareSlotsの効き方を検証しやすい。
  - シードキー（mapGroup/mapNum/area/slot、trainerId/partySize/slotなど）は固定運用。ビルド済みテーブル基準のスロット番号でログも合わせる。
  - ログ項目例:
    - 野生: mapGroup/mapNum, areaMask, slotIndex, isRareHit, finalSpeciesId, seed, slotCount
    - 釣り: 上記＋rodType(old/good/super)
    - トレーナー: trainerId, partyIndex, isRareHit(使うなら), finalSpeciesId, seed
    - レア抽選: rareHits（抽選で何枠当たりか）, rareAllocated（実際に確保した枠数）
- ログの有効条件:
  - デフォルトはOFF（通常ビルドでは出さない）。
  - DEBUGビルドまたはFLAG（例: FLAG_RANDOMIZER_DEBUG_LOG）で明示的にONにする。
  - さらに詳細なログが必要なら別FLAGで分離（例: VERBOSE用）を検討。
  - ログ出力形式: 既存のLOG_HANDLERに合わせたテキスト出力を想定（例: コンソール/デバッグログ）。本番ビルドでは出さない前提。
- 生成スクリプトの入出力（想定）:
  - 入力: `area_rules.yml`（レア枠メタ、maxSpecies、apply/removeなどを含む）
  - 出力: `generated/randomizer_area_rules.h`（再構成済みスロット/ルールを含むテーブル）
  - スクリプト配置想定: `dev_scripts/build_randomizer_area_rules.py`（例示）
  - 方針: ビルド前にスクリプト実行、生成物をコミット運用 or ビルド時必須実行のどちらかを選択
  - Land/Water/Rockの優先: それぞれエリア共通のrare指定をそのまま使う（竿別のような下位優先はなし）。もし種別ごとのrare指定を許すなら、種別優先＞エリア共通と明記する。
- 釣りのrareRateガイド
  - 基本は竿別に個別指定（エリア共通と併記した場合は竿別優先、共通は無視）。Landやエリア共通のrare指定は釣りには波及させない前提。
  - 例: Old=20〜30%、Good=40〜60%、Super=60〜80%など段階的に上げて攻略性を持たせる。
  - rareSlotsが上限として効き、rateが高くても枠数を超えて確保しない。逆にrateが低いとレア枠0もあり得る。
- 二重指定の優先度（案）
  - 基本方針: 矛盾する二重指定はビルドエラーで止める。
  - rareSlots vs rareRate: 併記は禁止（ビルドエラー）。運用を簡潔にするなら、どちらか一方に仕様を統一しても良い。どうしても併記を許すなら「rareSlots優先、rareRateは無視」のように固定し、警告を出す。
  - maxSpeciesOverrides: overridesがあれば優先。上限超過はエラー。
  - slotMode: エリア内で複数slotModeが書かれていたらエラー（上書きルールは設けない）。

ランタイム上書き（検討）
- 当面は導入しない（データ定義どおりの挙動を基本とする）。テストは明示的に用意したエリアデータで検証する。
- 将来必要になった場合のみ再検討（全域強制は破壊力が大きいためデフォルト封印）。
- 進化段階/BST帯のミックス: 基本はWLで許可したポケモンだけを出す設計で調整する（全ポケモン初期BL→WL許可）。序盤に高進化が混じるなどのバランスは、エリアごとのWL調整で対応し、maxSpecies側では特にフィルタを強制しない想定。
- スロット上限超えの指定: maxSpeciesが種別のスロット上限を超えていても実際には増えない（種別デフォルトでクランプ）。この矛盾はビルド時に警告を出しつつクランプするか、エラーで止めるかを決めておく（要検討）。
- スロット上限超えの運用決定: 当面はクランプ＋ビルド警告で進める（スロット上限に合わせて自動的に下げるが、警告は出す）。
- スロット上限超えの最終方針: maxSpeciesが種別上限（Land=12, Water/Rock=5, Fishing=2/5/5）を超えた場合はビルドエラーで落とす。クランプはしない。
- レア枠＋一般枠の合計: レア枠を先に確保し、その分一般枠の割当スロットを減らす（レア枠がスロット上限内で優先）。レア枠＋一般枠が上限を超えた場合は一般枠側を削る前提。
- maxSpecies=0指定時の扱い: その種別は出現なし（スロット割当ゼロ）。このとき rareSlots/rareRate が指定されていても無効とし、矛盾としてビルドエラーにする方針。
- スロット再構成とスロット数: 現行スロット数（Land=12, Water/Rock=5, 釣りは竿ごとに2/5/5）を据え置き、再構成時にスロット数を増減しない方針。スロット数を変えたくなる場合は別途明示的な拡張（改修）として扱う。
- maxSpecies指定がスロット数未満の場合: 使う種はmaxSpeciesまでにクランプし、残りスロットは割り当てない（Noneを置いても出現させない扱い）。スロット数自体は固定のまま。LandでmaxSpecies=5なら12スロットのうち5種のみ有効、残りは無効。
- maxSpeciesがスロット数（上限）を超える場合: ビルドエラーとする（例: Landで13以上を指定したらエラー）。
- スロット実体の削減（None防止）: maxSpeciesでスロット数未満を指定した場合、実際に「有効スロット数＝maxSpecies」に縮めて残りスロットを持たない（Noneで？アイコンが出る状況を排除する）。Landを5種にするならスロットも5件に再構成し、6〜12枠目を生成しない。
- 枠削減の具体例: maxSpecies=5のLandエリアなら、生成テーブルは5スロットのみを出力し、現行の12枠構造に0や未定義を残さない。6〜12枠目は物理的に存在しないので、??アイコンやダミーポケモンは出ない設計。
- maxSpeciesとスロット再構成: 枠を削った場合はビルド時にスロットテーブルを再構成し、実体スロットをmaxSpecies本数に固定する（余剰スロットなし）。シードやログでスロット番号を使う場合は新スロット数（例: 0〜maxSpecies-1）に合わせて扱う。スロット数を増やす方向（上限超え）はビルドエラー。
