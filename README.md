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

## Parameters
| Parameter | Size | Type | Description |
|-----------|------|------|-------------|
| FEE | 8 bytes | XFL LE | Optional fee setting 0-5% (0.00 - 0.05). Default 0.1%. Used for fee voting, during deposit, withdrawal and initial setup |
| WDR | 8 bytes | XFL LE | Optional withdrawal amount in LP tokens. Must be between 1% (0.01) and 99% (0.99) of holdings for partial withdrawals. If omitted then full withdrawal. |

## State Entries
| Key | Size | Type | Description |
|-----|------|------|-------------|
| "CUR" | 80 bytes | Raw bytes | Currency identifiers for pool tokens |
| | | | Bytes 0-39: Currency A (20 bytes currency code + 20 bytes issuer or zeros for XAH) |
| | | | Bytes 40-79: Currency B (20 bytes currency code + 20 bytes issuer or zeros for XAH) |
| "A" | 8 bytes | XFL LE | Current amount of currency A in pool |
| "B" | 8 bytes | XFL LE | Current amount of currency B in pool |
| "G" | 8 bytes | XFL LE | Geometric mean constant (A×B) |
| "TOT" | 8 bytes | XFL LE | Total LP tokens in circulation |
| "FAC" | 8 bytes | XFL LE | Fee accumulator tracking weighted preferences |
| `[Account ID]` | 16 bytes | XFL LE × 2 | Per-user state |
| | | | Bytes 0-7: User's LP token balance |
| | | | Bytes 8-15: User's fee preference |

## Notes
- XFL LE means [XLS-017](https://github.com/XRPLF/XRPL-Standards/discussions/39) in little-endian format.
- XAH (native XRP) amounts are represented with zero-filled issuer and currency fields
- All numerical values (amounts, fees, etc.) use the XFL format for consistent mathematical operations
- To retrieve the current fee: divide FAC by the TOT, keeping in mind these are both XFL LE so must be converted to decimal first.
