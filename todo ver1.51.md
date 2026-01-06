Daxnav を開き、Register に登録まではできる。その後登録したポケモンをRボタンで押下しても、揺れるどころか、揺れる判定のメソッドに到達していないことがわかる。
実際、Daxnav の UI画面から直接ポケモンお選択することで、草むらを揺らすことが成功しているためです。
また、それをやった後に Rボタンで再度登録し、Rボタンを押下すると、それ以降はフィールド画面からでも、Rボタンを押下して、草むらを揺らすことができる。
Rボタンを押下すると、ログも出るが、揺れる判定していないときは、ログもでない。

別件として、草むら揺らした後に、歩くと仕様通り、The Pokemon got away! のメッセージが出るか、ここの揺れる草むらが終わせないようにしたいが、実装しなかったっけ？
deha


• 調査メモ（コード変更なし）:

  - R ボタン押下で揺れないのは、TryStartDexNavSearch が動いていないか、Task_InitDexNavSearch ですぐ弾かれているケース。弾き条件は DexNavTryGenerateMonLevel が MON_LEVEL_NONEXISTENT を返すか、TryStartHiddenMonFieldEffect が失敗した時 (dexnav.c:962 付近)。
  - ランダマイザー産の種はバニラの野生表に無いので、ここでレベル決定に失敗する可能性が高いです。GetEncounterLevelFromMapData 冒頭の TryGetRandomizerDexNavLevel でレベルが拾えないと MON_LEVEL_NONEXISTENT になり、揺れ処理まで到達しません。
  - TryGetRandomizerDexNavLevel は RandomizerGetAreaRuleView を通せた場合のみ成功します。ここが失敗するとログも出ません（ログは RandomizerGetAreaRuleView 内のもの）。今回の「ログが出ない」状態はこの関数が FALSE を返しているサインです。
  - 登録 → フィールドで R では失敗し、UI から直接 A で揺らした後は R でも動く、という挙動は「初回はレベル決定に失敗してバイルート（NotFound）で終わっている」シナリオと一致します。UI から A で始めた検索は Task_DexNavExitAndSearch 経由で強制的に起動するため、そこでレベルが決まっ    た後は Var/Flag が更新され、以降の R 押下も通る、という流れが考えられます。

  確認すると良いポイント（ログやデバッグで値を観察するだけで済みます）:

  1. R 押下前に VarGet(DN_VAR_SPECIES) が 0 でないことを確認する。
  2. R 押下時に TryStartDexNavSearch が呼ばれているか（DN_FLAG_SEARCHING が立っていないか）。
  3. DexNavTryGenerateMonLevel の戻り値と、内部で TryGetRandomizerDexNavLevel が TRUE/ FALSE どちらになっているか。ここで FALSE なら今回の症状と一致します。
  4. RandomizerGetAreaRuleView に渡している area/rod/timeSlot が Route102 で有効な組み合わせか（ログが出ない場合はここで落ちている）。

  別件の「The Pokemon got away! を出さずに揺れを終わらせない」挙動は、Task_DexNavSearch の if (gTasks[taskId].tFrameCount > DEXNAV_TIMEOUT * 60) および距離オーバー時の EndDexNavSearchSetupScript(EventScript_PokemonGotAway, ...) がトリガーになっています。現行コードではタイムア
  ウト／距離離脱で必ずこのスクリプトに落ちる仕様のままです。
