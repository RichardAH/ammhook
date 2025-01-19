# AMM Hook
This Hook provides for a classic xy=k style AMM on an account on Xahau.

## Features
* Deposit (remit two currencies)
* Trade (remit one currency)
* Withdrawal (remit no currency)
* Fee voting (XFL LE "FEE" HookParameter when performing a Deposit)

## Installation
There are no install-time parameters for this Hook, simply install it on an account.

## Setup
To setup the AMM, Remit two currencies in the ratio that matches the initially desired price. This is the same as a deposit operation.
Optionally include a FEE HookParameter on this remit to set your desired AMM fee. This is an XFL LE between 0 and 0.05 (5%).

## Deposit
To deposit liquidity into the AMM Remit the two currencies the AMM contains in the correct ratio. To discover these check the state keys A and B, the data for which is XFL LE. Specifying an XFL LE between 0 and 0.05 as a HookParameter in this remit changes the LP holder's fee vote.

## Withdraw
To withdraw send an empty remit. 100% of the LP's share of the two currencies will be remitted back to them. To withdraw less than 100% of the user's LP tokens, submit with HookParameter "WDR" containing the number of LP tokens to liquidate.

## Trade
To trade against the AMM remit one of the two currencies to receive an emitted remit of the other.
