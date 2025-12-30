# TODO ver1.21

- 釣りのslotMode整合性: トップレベルslotModeがuniformのときでも、rodごとにslotMode: rareを許容してしまっている。仕様を決めてバリデーションを強化する（例: 釣りは常にrod別指定のみを有効にし、トップレベルslotModeを無視する or rod側がrareならトップもrare要求など）。
- レア枠定義漏れチェック: slotMode=rare/rareSlots>0なのに対象時間帯・rodに適用するポケモン定義(WL)が無いケースを検出してエラー/警告にする。現在の仕様では「レア枠を作ったが定義が無い」場合の挙動が曖昧。
- 釣りWLのrod分離対応: 現行は時間帯ごとにWLが共通で、rod別に伝説可否/適用キットを分けられない。superだけ伝説、old/goodは非伝説といった運用ができるよう、データ構造と生成・ランタイムを拡張する方針を決めて改修する（改修必須）。rod内にremove legend_unlock、superにapply legend_unlockを記載しても共通WLにマージされるため成立せず、old/goodのremoveで共通WLから伝説が消えたり、superのapplyが共通WLに混ざりold/goodでも伝説が出る等の副作用が出る。

## 設計メモ（案）
- slotMode整合性:
  - 採用: 方針A。トップレベルのslotModeを必須・rod側はslotMode省略可（省略時はトップを継承）。トップ=uniformならrodでrareを禁止（エラー）。トップ=rareならrodもrareのみ許容。

- レア枠定義漏れチェック:
  - slotMode=rareまたはrareSlots>0のとき、当該スコープ（共通/時間帯/rod）で最終的なWL件数が0ならエラー、希少枠だけ0件で通常枠がある場合は警告など粒度を決める。
  - rareRate>0かつrareSlots>0なのにapply/remove後のtargetCountが0の場合も検出。
  - データ設計: 通常枠/レア枠ともに配列はNoneまたは1件以上のみ許容。空配列はエラー。レア枠用には通常とは別のapply/remove（例: rareApply / rareRemove）を用意。slotMode=rareでレア枠が未定義(None)ならエラー。rare枠がNoneならrareRate/rareSlotsもNone（未指定）扱いにする。
  - rareSlots=0なのにslotMode=rare/rareApply/rareRemoveを指定した場合も設計ミスとしてエラー。rareを使わないならslotModeをuniformにする前提。
  - rareRate/rareSlotsは明示必須（rare枠がNoneなら未指定扱いでOK）。マイナスや上限超過はエラー。
  - slotMode=rareのとき、timeSlotsを使う場合はどこかのスロットでrareRate/rareSlotsがセットで定義されていることが必須。全スロットでnullのままならビルドエラー。
  - allowEmptyフラグをスコープ全体に用意し、apply/remove後に空になる場合はデフォルトエラー。意図的に空（出現ゼロ）にしたいときだけ allowEmpty: true を明示。timeSlots未指定（ANY）も含め、allowEmptyなしで空ならエラー。allowEmpty=trueならrareSlots/rareRateをNone扱いとし、0指定はエラー。
  - rareRate=0またはrareSlots=0でslotMode=rareの場合は設計ミスとしてビルドエラー（rareSlots>0かつrareRate>0を要求）。
  - rareRate/rareSlotsの範囲: intでrareRateは0〜100に制限（負数もエラー）。rareSlotsは種別スロット上限まで（負数もエラー）。rareRate=100ならレア枠を最大数まで埋める。rareRate=30ならコード側で抽選し30%の確率でレア枠を確保（一般枠をスロット数に応じて消費）。スロット上限も考慮してrareSlotsを超えないようにする。
  - slotMode=uniformでrareRate/rareSlotsが指定されていたらエラー（rareを使わないなら指定禁止）。

- rod分離:
  - データ構造を「時間帯→rod→専用WL/BL」に変更し、rodごとに独立したapply/removeを持つ（共通WLにマージしない）。
  - 生成物：FishingRuleに「WL/BLのoffset/length」を追加、RandomizerAreaRuleにもrod別参照を持たせる。
  - ランタイム：SelectFishingRuleでrod専用のWL/BLを参照し、allowLegendOverrideもrod/時間帯ごとに判定。現行の共通WLフォールバックを廃止。
  - 互換性: 既存データは移行スクリプトで「共通→全rodに複製」する運用を用意。
  - allowLegendOverrideの継承は共通→timeSlot→rod（分離後）で固定。
  - BL設計: 通常/rareともにremove（BL）で統一（default/rareそれぞれにapply/removeを持つ）。
  - 旧フォーマット（フラットなapply/remove等）は非サポート。新フォーマットに書き直す前提とし、旧形式のバリデーションは不要（使用時はエラー扱いで構わない）。
  - 生成物フィールド構造（案→詳細化）:
    - generated/randomizer_area_rules.h に struct RandomizerAreaRule を出力。従来の mapGroup/mapNum/areaMask/whitelist/blacklist/maxRerolls/allowLegendOverride/slotMode/maxSpecies/rareRate/rareSlots に加え、釣り用に FishingRule 配列へのポインタと rod count を持たせる（例: fishingRules, fishingRuleCount）。Land/Water/Rockは fishingRules=null, count=0。
    - struct RandomizerFishingRule を新設: normalWlOffset/normalWlCount、normalBlOffset/normalBlCount、rareWlOffset/rareWlCount、rareBlOffset/rareBlCount、slotMode、maxSpecies、rareRate、rareSlots、allowLegendOverride を保持し、各rod（old/good/super…）ごとに1要素。
    - WL/BLは共通の pool 配列（const u16 gRandomizerAreaSpeciesPool[] など）にまとめ、offset/lengthで指す。rare/normalでプールを分けてもよいが、単一プール＋セグメント管理を想定。
    - timeSlotsを使う場合は AreaRule を timeSlotごとに分割生成（mapGroup/mapNum/areaMask/timeSlotキーをdata1に詰める想定）。timeSlots=false/anyの場合はtimeSlot=0xFF固定で1エントリ。
    - これらフィールド名をデータ側にコメントで残し、スクリプトが不足フィールドを検出したら mapGroup/mapNum/areaMask/timeSlot/rod を含むエラーを出す。

- YAML構造:
  - default/rareをネストで固定（例: default.apply/remove、rare.apply/remove）。timeSlots/rodでも同じ構造を必須化。
  - allowEmptyはtimeSlot単位で1つだけ指定（rod単位にはしない）。default/rareは共通管理（別フラグは設けない）。
  - timeSlotsの扱い: 利用しない場合も `timeSlots: false` を必須記載。OW_TIME_OF_DAY_ENCOUNTERSがTRUEならtimeSlot=None(ANY)として扱う。OW_TIME_OF_DAY_ENCOUNTERSがFALSEでもtimeSlotsを記載し、FALSEで固定する運用に統一。TRUEなのにFALSEを指定したらビルドエラーでOW_TIME_OF_DAY_ENCOUNTERS設定を参照するメッセージを出す。OW_TIME_OF_DAY_ENCOUNTERS=FALSE時のtimeSlot値は0xFF固定。
  - timeSlotsが有効（false以外）の場合、トップレベルのdefault/rareは記載不可。timeSlotブロック内でdefault/rareを必須記載する。トップに書いていたらバリデーションエラー。
  - timeSlots=falseの場合でもslotMode=rareならrareRate/rareSlots/rareApplyをnull以外で設定してもよい。slotMode=uniformの場合はrareRate/rareSlots/rareApplyの記載自体を必須としつつ、値はnullのみ許容（null以外ならエラー）。
  - OW_TIME_OF_DAY_ENCOUNTERS=TRUEなのにtimeSlots:falseを指定した場合はバリデーションエラー（設定値を参照させる）。
  - 全時間帯同一設定にしたい場合はtimeSlots.anyを記載する（falseは禁止）。 OW_TIME_OF_DAY_ENCOUNTERS=FALSEでもanyは便宜上許容するがtimeSlot=0xFF固定。

- デフォルト値の扱い:
  - maxSpecies/slotMode/rareRate/rareSlots/maxRerollsは明示必須（省略不可）。省略時は「どのキーが不足か」を含めてビルドエラーを出す。

- maxSpecies / rareSlots / スロット上限の矛盾:
  - クランプは行わず、allowEmptyなしで矛盾（上限超過/0スロット）が出たらエラー。空を許容したい場合のみallowEmptyで明示して0スロットを許可。Noneは使わない。
  - スロット上限は定数化（例: LAND_MAX_SLOTS / WATER_MAX_SLOTS / ROCK_MAX_SLOTS / ROD_MAX_SLOTS 等）し、maxSpeciesやrareSlotsはこれらを参照して判定する。
  - maxSpeciesが種別上限（例: LandならLAND_MAX_SLOTS=12、Water/Rock等も同様の定数）を超えたらエラー。allowEmpty=trueの場合は0にしたいのでmaxSpeciesはnullのみ許容。

- allowEmptyとmaxRerolls:
  - allowEmpty=trueの場合はmaxRerollsをnull指定（未指定）も許可し、指定があればその値を採用。ただし0はエラー（意図的に空ならnullで表現）。rareRate/rareSlotsは未指定(null)とし、0指定はエラー（空運用時はrare抽選を発生させない）。

- エラーメッセージ粒度:
  - エラー/警告にはmapGroup/mapNum/areaMask/timeSlot/rodを含め、どのキーが不足/矛盾かを明示。stderrで出力。

- ログ仕様:
  - rareヒット時に「rare pool使用」をログに出せるようにする（デバッグフラグ連動で可視化）。

- テスト:
  - 可能であればスクリプトにyaml→header出力の単体テストを追加し、主要なバリデーションケース（不足キー、負数、上限超過、timeSlots=falseでのrare指定など）をカバーする。

## 改修優先順位（推奨）
1. レア枠定義漏れチェック（バリデーション強化）でデータ事故を防ぐ。
2. rod分離の仕様決定・実装（根本課題の解消）。データ構造とランタイム改修。
3. slotMode整合性の締め（トップ/rod混在ルールの厳格化）。rod分離後にバリデーションを入れると混乱が少ない。

## 実装方針（rod分離を含む段階的アプローチ）
- 段階A: バリデーション強化のみ先行
  - 現行フォーマットのまま、slotMode/rareRate/rareSlots/maxSpecies/maxRerolls/timeSlotsの必須化と矛盾チェックを厳格化。
  - rare枠未定義、空配列、負値、上限超過、slotModeとの不整合をエラー化。
  - エラーメッセージに mapGroup/mapNum/areaMask/timeSlot/rod を含める。
- 段階B: 生成物フォーマットを rod 分離版に切り替え
  - FishingRuleを新設し、WL/BLをrod別にoffset/lengthで指す構造に移行。
  - timeSlot有効時はAreaRuleをtimeSlotごとに生成（timeSlot=0xFFでANY/falseを表現）。
  - allowLegendOverride等の継承順を共通→timeSlot→rodで固定。
  - 旧データはスクリプト側で共通WLを各rodに複製する移行ステップを用意。
- 段階C: ランタイム導線の置き換え
  - SelectFishingRuleがrod別FishingRuleを参照するように変更し、共通WLフォールバックを廃止。
  - rare抽選時にrare pool使用をログに出せるようにする（デバッグフラグ連動）。
- 段階D: データキット/テンプレ整備
  - rare用テンプレ（apply/remove）とデフォルト用テンプレを分け、apply/remove後の空をallowEmptyで明示。
  - サンプルYAMLを「timeSlots無し」「timeSlots有り」「rod分離」「allowEmpty」「rare有り/無し」など複数パターンで提示し、スクリプト内テストにも流用。

## 追加で詰めたい懸念点
- areaMask/timeslot/rodのキー固定リストを明文化: areaMaskは Land/Water/Rock/Fish 固定、timeslotは morning/day/evening/night/any、rodは old/good/super。バリデーションのエラーメッセージにもこれらを出す。
- スロット上限定数の置き場所と名称: LAND_MAX_SLOTS / WATER_MAX_SLOTS / ROCK_MAX_SLOTS / ROD_MAX_SLOTS をどのヘッダで持つか（例: include/constants/randomizer_slots.h）を決める。スクリプト側でも同じ値を参照できるよう一元管理。
- 生成物の出力先ファイルとinclude先: generated/randomizer_area_rules.h / .c などに確定し、ビルドで必ずインクルードされるように make ルールを記述する。FishingRuleも同ファイルで出力する方針を明記。
- 旧データの移行手順: 共通WLをold/good/superへ複製する簡易スクリプトを用意し、移行時に重複除去・ソート・上限チェックも行う。データ膨張によるプールサイズやROMサイズの影響を事前に見積もる。
- バリデーションのテスト観点を追加: timeSlots=falseでslotMode=rareかつrareRate/rareSlots指定、allowEmpty=true時のmaxRerolls null許容、slotMode=uniformでrare指定が残っていないか、maxSpecies/rareSlotsの上限超過、rareRate/rareSlots負値、rareRate/rareSlots部分指定（片方だけ）などをスクリプトテストに入れる。
- 固定キーリストの運用: areaMaskは Land/Water/Rock/Fish 固定、timeslotは morning/day/evening/night/any、rodは old/good/super に限定。これ以外はバリデーションエラー。欠落もエラー。スクリプト内で文字列→定数のマップを一元管理し、生成物はtimeSlot=0x00/0x01/0x02/0x03/0xFF等の固定値で出力（areaMask/rodも同様）。YAMLサンプルにも「固定キーのみ有効」を明記する。
- timeSlots=falseとanyの扱い: OW_TIME_OF_DAY_ENCOUNTERS=TRUEなのにtimeSlots:falseはエラー。全時間帯共通にしたい場合はanyを記載し、default/rareをきちんと埋める。anyはTRUE時は共通スロット、FALSE時は0xFF固定扱い。
- allowEmptyとslotMode=rareの組み合わせ:
  - timeSlots配下: allowEmpty=trueでもslotMode=rare自体は例外的に許容。ただしrareRate/rareSlotsがnullのまま（定義なし）ならエラー。rare枠を空運用する場合でも、rareRate/rareSlotsはセットで非nullを要求（0はエラー）。欠落はバリデーションエラー。
  - グローバル（timeSlots=false/any）: allowEmpty=true かつ slotMode=rare は非推奨のためエラーにする（矛盾扱い）。rareを使わないならslotMode=uniformにする前提。
- プールオフセットの型と上限チェック: 生成物のoffset/lengthはu32で保持し、スクリプト側でpool lengthおよびoffset+countがu16上限を超えた場合はエラー。ランタイムはu32のまま読むか、チェック済み前提でu16に落として使う。
- ソートと重複除去: apply/remove後のWL/BLは昇順ソート＋重複除去を仕様で固定（rare/normalとも同じ処理）。これにより生成物のoffset/lengthとプールの内容が決定的になり、デバッグ比較が安定する。重複があっても自動除去される前提でバリデーションはエラーにしない。
- maxRerollsのnull扱い: allowEmpty=true時のみnullを許容。それ以外（通常運用）でnullが書かれていたらエラーとする。0もエラー（空運用はallowEmpty+nullで明示）。
- ログレベルの既定: rareヒットログやデバッグ出力はWARNで出す。INFOでは実機で埋もれるため、バリデーション警告もWARN統一。
- any→false移行のガイド: 旧データでtimeSlots:falseを使っている場合はanyに書き換えて共通設定を明記するのを推奨（OW_TIME_OF_DAY_ENCOUNTERS=TRUEではfalseはエラー）。移行はスクリプトで一括置換し、必須キー不足がないかバリデーションする。
- rareRate/rareSlotsの部分指定: 片方だけ指定はエラー。rareを使う場合はrareRateとrareSlotsをセットで必須。どちらもnullなら「未使用」、どちらも非nullで有効、混在はバリデーションエラー。
- rareRate/rareSlotsの境界: rareRateは1〜100の整数（0は未使用扱いでエラー）。rareSlotsは1〜種別上限まで。rareRate=100なら可能な限りレア枠を埋める挙動となり、rareSlots=maxも利用可（例: 12枠ベースでrareRate=50, rareSlots=maxなら50%の試行でレア枠を確保し、最大枠まで埋まる可能性もある）。上限超過はエラー。
- allowLegendOverrideの継承: 共通→timeSlot→rodの一方向で上書き可。normal/rareで分けず同一値を使う（片方だけ指定といった分岐は許容しない）。
- 伝説を含むレア枠の扱い: rare.applyに伝説系が含まれる場合は、同スコープでallowLegendOverride=trueを必ず明示する運用に固定（自動ONは行わず、指定漏れをバリデーションで警告またはエラーにする）。意図しない伝説解禁を防ぎつつ、レア枠で解禁する設計を明示する。
- allowLegendOverride=trueでレア枠が伝説のみの場合: 正常ケースとして許容（意図的な「伝説専用レア枠」とみなす）。rare.applyが空なのにallowLegendOverride=trueならエラー（allowEmptyなら例外的に可）。allowLegendOverride=trueなのにレア枠に伝説が1体も無い場合は警告またはエラー（データ設計ミスを検知する）。
- allowLegendOverride=falseで伝説を含む場合: 伝説はフィルタで除去される前提。除去後に通常枠/レア枠が空になったら警告（allowEmpty=trueなら0スロットを許容し、出現なしにする）。「一般枠が無く伝説だけ」の設計ミスを警告で検知する。
- timeSlotコード値一覧（生成物）: morning=0, day=1, evening=2, night=3, any/false=0xFF。生成物のtimeSlotフィールドにはこの固定値を出力し、デバッグログやエラーメッセージも同値で報告する。
- slotMode=rareで全timeSlotのrareRate/rareSlotsがnullの場合: エラー（rareを使うと宣言したのに全スロット未定義）。最低1スロットではrareRate/rareSlotsを非nullで定義することを要求。
- OW_TIME_OF_DAY_DISABLE_FALLBACKとの連携: 空WLで出現ゼロになる場合はWARNログを出す（allowEmpty=trueなら想定内）。FALSE運用でtimeSlotsが空→共通にフォールバックする挙動は無しで、空は警告にする。
- テストデータの配置/命名: サンプルYAMLやバリデーション用テストデータは dev_scripts/tests/randomizer_yamls/ に置く方針。timeSlots無し/有り/rod分離/allowEmpty/rare有無/エラーケースなどを網羅してスクリプトテストに流用。
- データ入力コメントガイド: YAML冒頭に固定キー（areaMask/timeslot/rod）、定数（*_MAX_SLOTS）、slotMode/rareRate/rareSlots必須、timeSlots=false禁止（TRUE環境）などの注意書きをコメントで残す。
- rare用テンプレ命名: rare専用キットは意図が分かる名称（例: legend_unlock, rare_water_X）を推奨（強制はしないがコメントで推奨）。
- timeSlotsで空継承させたい場合: timeSlotブロックを空オブジェクト({})にはせず、allowEmpty:trueを明示し、default/rareやfishing配下をnullで埋める（maxRerollsもnull）。暗黙の継承空欄はバリデーションエラーにする。
- rodごとのmaxSpecies/maxRerolls: timeSlots配下でfishingを持つ場合、各rodにmaxSpecies（スロット上限まで）とmaxRerollsを必須とする。allowEmpty:trueで空を許容する場合のみnullを許容（0はエラー）。上位の値を継承する暗黙ルールは設けず、明示記載を要求。
- fishing未記載の扱い: areaMask=Fishでfishingブロックが無い、またはold/good/superのいずれかが欠落している場合はエラー。Land/Water/Rockではfishing未記載は無視可。

## サンプル（新フォーマット例）
```yaml
areas:
  route_103_fishing:
    key: { mapGroup: MAP_GROUP(MAP_ROUTE103), mapNum: MAP_NUM(MAP_ROUTE103), areaMask: Fish }
    slotMode: rare
    maxSpecies: FISH_MAX_SLOTS
    maxRerolls: 6
    allowLegendOverride: false
    timeSlots:
      morning:
        default: { apply: [starter_water], remove: null }
        rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
      day:
        allowEmpty: true
        default: null
        rare:    null
      evening:
        allowEmpty: true
        default: null
        rare:    null
      night:
        allowLegendOverride: true
        default:
          apply: [starter_water]   # 夜の通常枠
          remove: null
        rare:
          apply: [legend_unlock]   # 夜のレア枠
          remove: [starter_water, late_water]  # 共通適用を落とす
          rareRate: 100
          rareSlots: 1
    # rod分離後の例（イメージ）
    fishing:
      old:
        default: { apply: [starter_water], remove: null }
        rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
      good:
        default: { apply: [starter_water, mid_water], remove: null }
        rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
      super:
        allowLegendOverride: true
        default: { apply: [starter_water, late_water], remove: null }
        rare:    { apply: [legend_unlock], remove: [starter_water, late_water], rareRate: 100, rareSlots: 1 }
```

## サンプル追加（パターン集）
1) シンプル（timeSlotsなし・rareなし）
```yaml
areas:
  route_101_land:
    key: { mapGroup: MAP_GROUP(MAP_ROUTE101), mapNum: MAP_NUM(MAP_ROUTE101), areaMask: Land }
    slotMode: uniform
    maxSpecies: LAND_MAX_SLOTS
    maxRerolls: 6
    allowLegendOverride: false
    default: { apply: [starter_land], remove: null }
    rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
    timeSlots: false  # OW_TIME_OF_DAY_ENCOUNTERS=FALSE環境ならfalse/anyいずれも許可。TRUEではfalseはエラー（共通はanyで明記）。
```

2) timeSlotsあり・夜だけrare（伝説解禁）※空継承はallowEmptyで明示
```yaml
areas:
  route_103_fishing:
    key: { mapGroup: MAP_GROUP(MAP_ROUTE103), mapNum: MAP_NUM(MAP_ROUTE103), areaMask: Fish }
    slotMode: rare
    maxSpecies: FISH_MAX_SLOTS  # 釣り用定数に置換
    maxRerolls: 6
    allowLegendOverride: false
    timeSlots:
      morning:
        default: { apply: [starter_water, late_water], remove: null }
        rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
      day:
        allowEmpty: true
        default: null
        rare:    null
      evening:
        allowEmpty: true
        default: null
        rare:    null
      night:
        allowLegendOverride: true
        default: { apply: [starter_water], remove: null }
        rare:    { apply: [legend_unlock], remove: [starter_water, late_water], rareRate: 100, rareSlots: 1 }
```

3) allowEmpty=trueで出現ゼロを許容（例: 夜だけ出現なし）
```yaml
areas:
  route_999_hidden:
    key: { mapGroup: 0xFF, mapNum: 0xFF, areaMask: Hidden }
    slotMode: uniform
    maxSpecies: LAND_MAX_SLOTS  # Land用定数を使用（Water/Rockは各定数に置換）
    maxRerolls: 6
    allowLegendOverride: false
    timeSlots:
      morning:
        default: { apply: [starter_land], remove: null }
        rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
      day:
        default: { apply: [starter_land], remove: null }
        rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
      evening:
        default: { apply: [starter_land], remove: null }
        rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
      night:
        allowEmpty: true     # 夜は出現ゼロを許容
        maxRerolls: null     # allowEmpty併用時はnull可（0はエラー）
        default: { apply: null, remove: null }
        rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
```

4) rod分離の例（superだけrare、old/goodは通常）
```yaml
areas:
  route_200_fishing:
    key: { mapGroup: 2, mapNum: 10, areaMask: Fish }
    slotMode: rare
    maxSpecies: FISH_MAX_SLOTS
    maxRerolls: 6
    allowLegendOverride: false
    timeSlots:
      morning:
        fishing:
          old:
            default: { apply: [starter_water], remove: null }
            rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
          good:
            default: { apply: [starter_water, mid_water], remove: null }
            rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
          super:
            allowLegendOverride: true
            default: { apply: [starter_water, late_water], remove: null }
            rare:    { apply: [legend_unlock], remove: [starter_water, late_water], rareRate: 100, rareSlots: 1 }
      day:
        allowEmpty: true
        fishing:
          old:   { default: null, rare: null, maxRerolls: null }
          good:  { default: null, rare: null, maxRerolls: null }
          super: { default: null, rare: null, maxRerolls: null }
      evening:
        allowEmpty: true
        fishing:
          old:   { default: null, rare: null, maxRerolls: null }
          good:  { default: null, rare: null, maxRerolls: null }
          super: { default: null, rare: null, maxRerolls: null }
      night:
        allowEmpty: true
        fishing:
          old:   { default: null, rare: null, maxRerolls: null }
          good:  { default: null, rare: null, maxRerolls: null }
          super: { default: null, rare: null, maxRerolls: null }
```

5) OW_TIME_OF_DAY_ENCOUNTERS=FALSE想定（timeSlot固定0xFF）
```yaml
areas:
  route_300_land:
    key: { mapGroup: 3, mapNum: 5, areaMask: Land }
    slotMode: uniform
    maxSpecies: LAND_MAX_SLOTS
    maxRerolls: 6
    allowLegendOverride: false
    # timeSlotsを書くが、OW_TIME_OF_DAY_ENCOUNTERS=FALSEなので0xFF固定で運用（falseは禁止、anyは便宜上許容するが非推奨）
    timeSlots:
      any:
        default: { apply: [starter_land, mid_land], remove: null }
        rare:    { apply: null, remove: null, rareRate: null, rareSlots: null }
```
※ timeSlots.anyはOW_TIME_OF_DAY_ENCOUNTERSがTRUE/FALSEいずれでも便宜上許容するが、FALSE時はtimeSlot=0xFF固定で運用（非推奨だが設計上可能）。
