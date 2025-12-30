# TODO ver1.3

- 釣りのslotMode整合性: トップレベルslotModeがuniformのときでも、rodごとにslotMode: rareを許容してしまっている。仕様を決めてバリデーションを強化する（例: 釣りは常にrod別指定のみを有効にし、トップレベルslotModeを無視する or rod側がrareならトップもrare要求など）。
- レア枠定義漏れチェック: slotMode=rare/rareSlots>0なのに対象時間帯・rodに適用するポケモン定義(WL)が無いケースを検出してエラー/警告にする。現在の仕様では「レア枠を作ったが定義が無い」場合の挙動が曖昧。
- 釣りWLのrod分離対応: 現行は時間帯ごとにWLが共通で、rod別に伝説可否/適用キットを分けられない。superだけ伝説、old/goodは非伝説といった運用ができるよう、データ構造と生成・ランタイムを拡張する方針を決めて改修する（改修必須）。rod内にremove legend_unlock、superにapply legend_unlockを記載しても共通WLにマージされるため成立せず、old/goodのremoveで共通WLから伝説が消えたり、superのapplyが共通WLに混ざりold/goodでも伝説が出る等の副作用が出る。
