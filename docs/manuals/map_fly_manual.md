# Map / Fly Manual

この manual は、新規マップ、タウンマップ表示、Fly 登録を扱うときの入口です。
詳細は [Map Registration Fly Region Flow](../flows/map_registration_fly_region_flow_v15.md) を参照します。

## 先に分けるもの

新規マップ作業では、次が混ざりやすいです。

- 実際の map data。
- map name の表示。
- region map 上の座標。
- Fly destination。
- Fly 解放 flag。
- badge や gym clear の条件。
- 赤版、青版など version 差分。

一度に全部を変えると点滅、座標ズレ、Fly 先不一致の原因を追いづらくなります。
まずは map 登録、次に表示、最後に Fly の順に分けます。

## 作業フロー

1. 既存の近い town/city map を 1 つ選ぶ。
2. map group、map number、map name の参照を検索する。
3. 新規 map の登録箇所を既存例と同じ形式で追加する。
4. region map 座標と icon を追加する。
5. Fly destination を追加する。
6. Fly 解放 flag をどのイベントで立てるか決める。
7. version 差分がある場合は、赤版、青版で条件を分ける。

## 検索例

```sh
rtk rg -n "MAP_GROUP|MAP_NUM|MAPSEC_|FLYDEST|HEAL_LOCATION" include src data
```

実際の定数名は既存 map に合わせて検索します。
新規名だけで探す前に、成功している既存 map を起点にします。

## 点滅や表示不具合が出るとき

タウンマップや icon 更新後に Fly で点滅する場合は、次を疑います。

- region map の座標と Fly destination が同期していない。
- icon 更新と destination 更新のタイミングがズレている。
- version 差分の条件で片方だけ flag が立っている。
- mapsec 表示名と destination が別の entry を見ている。
- Fly 解放後に表示 refresh される順序が既存例と違う。

不具合を調べるときは、まず正常に動く既存町の登録一式を横に並べ、同じ種類の定義がすべて揃っているか確認します。
