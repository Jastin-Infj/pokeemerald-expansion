# TM/HM Manual

この manual は、TM/HM の追加とフィールド技整理の入口です。
250 前後まで TM を増やす計画は [TM/HM Expansion](../overview/tm_hm_expansion_250_v15.md) を先に確認します。

## TM/HM の定義場所

TM/HM の一覧は主に `include/constants/tms_hms.h` にあります。

```sh
rtk rg -n "FOREACH_TM|FOREACH_HM|FOREACH_TMHM" include/constants/tms_hms.h
```

TM/HM はアイテムとしても扱われるため、次も影響します。

- `include/constants/items.h`
- `src/data/items.h`
- bag pocket の TM/HM capacity
- teachable learnsets

## TM を増やすときの流れ

1. `include/constants/tms_hms.h` の `FOREACH_TM` に候補を追加する。
2. アイテム ID と bag 表示が想定通り増えるか確認する。
3. 該当技を覚えられる species を teachable learnsets に接続する。
4. 入手イベント、ショップ、報酬、説明文を追加する。
5. ROM 上で bag、技マシン使用、習得不可表示を確認する。

## 250 TM で注意すること

TM/HM を 250 前後まで増やすと、単なるリスト追加では済まない可能性があります。
特に次を先に確認します。

- アイテム ID の上限。
- held item や save に入る item width。
- bag の表示数、スクロール、capacity。
- relearn や teachable learnset の generated data。
- upstream 追従時の conflict。

## HM をキーアイテムやフラグ判定に寄せる場合

フィールド技を HM item 所持ではなく、キーアイテム、badge 数、story flag で解放する設計にする場合は、TM/HM 追加とは別の機能として扱います。

見る観点は次です。

- フィールド上で技を呼び出している script。
- party move check が必要か。
- badge 数や story flag の判定場所。
- UI 上にどう表示するか。
- 既存 HM item を残すか、互換表示だけにするか。

この設計はまだ固定しない段階なので、実装前に flow doc を追加します。
