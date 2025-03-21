#include "hookapi.h"
#define ttREMIT 95
#define MAX_EMIT_RETRIES 5
#define DONE(x)\
    return accept(x, sizeof(x), __LINE__);
#define NOPE(x)\
    return rollback(x, sizeof(x), __LINE__);
#define MAX_AMM_FEE 6057837899185946624ULL /* 5% */
#define COPY20(src,dst)\
{\
    uint32_t* x = (dst);\
    uint32_t* y = (src);\
    *x++ = *y++;\
    *x++ = *y++;\
    *x++ = *y++;\
    *x++ = *y++;\
    *x++ = *y++;\
}

#define XAH_TO_DROPS(xah)\
    float_multiply(xah, float_set(6, 1))

#define COPY40(src,dst)\
{\
    uint64_t* x = (dst);\
    uint64_t* y = (src);\
    *x++ = *y++;\
    *x++ = *y++;\
    *x++ = *y++;\
    *x++ = *y++;\
    *x++ = *y++;\
}

#define ISZERO20(src)\
(\
    *(((uint64_t*)(src)) + 0) == 0 &&\
    *(((uint64_t*)(src)) + 1) == 0 &&\
    *(((uint32_t*)(src)) + 4) == 0)

#define ENSURE_TRUSTLINE_EXISTS(cur, iss)\
{\
            uint8_t kl[34];\
            if (!BUFFER_EQUAL_20(iss, OTXNACC))\
            {\
                util_keylet(SBUF(kl), KEYLET_LINE, (iss), 20, OTXNACC, 20, (cur), 20);\
                if (slot_set(SBUF(kl), 50) != 50)\
                    NOPE("AMM: Destination account does not have the required trustline setup.");\
            }\
}

#define SVAR(x) &x, sizeof(x)

uint8_t txn_remit[60000] =
{
/* size,upto */
/*   3,   0 */   0x12U, 0x00U, 0x5FU,                                                           /* tt = Remit       */
/*   5,   3 */   0x22U, 0x80U, 0x00U, 0x00U, 0x00U,                                          /* flags = tfCanonical */
/*   5,   8 */   0x24U, 0x00U, 0x00U, 0x00U, 0x00U,                                                 /* sequence = 0 */
/*   6,  13 */   0x20U, 0x1AU, 0x00U, 0x00U, 0x00U, 0x00U,                                      /* first ledger seq */
/*   6,  19 */   0x20U, 0x1BU, 0x00U, 0x00U, 0x00U, 0x00U,                                       /* last ledger seq */
/*   9,  25 */   0x68U, 0x40U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U, 0x00U,                         /* fee      */
/*  35,  34 */   0x73U, 0x21U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,       /* pubkey   */
/*  22,  69 */   0x81U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                  /* srcacc  */
/*  22,  91 */   0x83U, 0x14U, 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                  /* dstacc  */
/* 138, 113 */   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* emit detail */
                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

/*   2, 251 */  0xF0U, 0x5CU,                                                               /* lead-in amount array */
/*   2, 253 */  0xE0U, 0x5BU,                                                               /*lead-in amount entry A*/
/*  49, 255 */  0x61U,
                0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,
                0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,
                /* amount A */
                0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,
/*   3, 304 */  0xE1, 0xE0U, 0x5BU,                                                         /*lead-in amount entry B*/
/*  49, 307 */  0x61U,
                0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,
                0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,      /* amount B */
                0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,0x99,
                0x99,0x99,0x99,0x99,0x99,0x99,
/*   2, 356 */  0xE1, 0xF1                                                 /* lead out, may also appear at end of A */
/*   -, 358 */                
};

#define TXN_CUR_A (txn_remit + 255)
#define TXN_CUR_B (txn_remit + 307)
#define OTXNACC (txn_remit + 93)
#define HOOKACC (txn_remit + 71)
#define TXN_EDET (txn_remit + 113)

#define BE_DROPS(drops)\
{\
    uint64_t drops_tmp = drops;\
    uint8_t* b = (uint8_t*)&drops;\
    *b++ = 0b01000000 + (( drops_tmp >> 56 ) & 0b00111111 );\
    *b++ = (drops_tmp >> 48) & 0xFFU;\
    *b++ = (drops_tmp >> 40) & 0xFFU;\
    *b++ = (drops_tmp >> 32) & 0xFFU;\
    *b++ = (drops_tmp >> 24) & 0xFFU;\
    *b++ = (drops_tmp >> 16) & 0xFFU;\
    *b++ = (drops_tmp >>  8) & 0xFFU;\
    *b++ = (drops_tmp >>  0) & 0xFFU;\
}

#define DEBUG 1 

// warning:
// the xflA and xflB populate only the emit stub, not the emit template
#define DO_REMIT(early, xflA, xflB, retry_count)\
{\
        int64_t bytes = 358;\
        if (early)\
        {\
            txn_remit[305] = 0xF1U;\
            bytes = 306;\
        }\
        etxn_details(TXN_EDET, 138);\
        trace(SBUF("to emit:"), txn_remit, bytes, 1);\
        int64_t fee = etxn_fee_base(txn_remit, bytes);\
        BE_DROPS(fee);\
        *((uint64_t*)(txn_remit + 26)) = fee;\
        int64_t seq = ledger_seq() + 1;\
        txn_remit[15] = (seq >> 24U) & 0xFFU;\
        txn_remit[16] = (seq >> 16U) & 0xFFU;\
        txn_remit[17] = (seq >>  8U) & 0xFFU;\
        txn_remit[18] = seq & 0xFFU;\
        seq += 4;\
        txn_remit[21] = (seq >> 24U) & 0xFFU;\
        txn_remit[22] = (seq >> 16U) & 0xFFU;\
        txn_remit[23] = (seq >>  8U) & 0xFFU;\
        txn_remit[24] = seq & 0xFFU;\
        trace(SBUF("emit:"), txn_remit, bytes, 1);\
        uint8_t emithash[32];\
        int64_t emit_result = emit(SBUF(emithash), txn_remit, bytes);\
        if (DEBUG)\
            TRACEVAR(emit_result);\
        if (emit_result < 0)\
            rollback(SBUF("AMM: Emit failed."), __LINE__);\
        {\
            uint8_t emit_stub[41]; /* 20 bytes of accid, 4 bytes of ledger seq, 8 bytes xflA, 8 bytes xflB, 
                                      1 byte retry count */\
            emit_stub[40] = (retry_count);\
            COPY20(OTXNACC, emit_stub);\
            *((uint32_t*)(emit_stub + 20)) = ledger_seq();\
            *((uint64_t*)(emit_stub + 24)) = xflA;\
            *((uint64_t*)(emit_stub + 32)) = xflB;\
            state(SBUF(emit_stub), SBUF(emithash));\
            uint32_t currently_pending_emits = 0;\
            state(SVAR(currently_pending_emits), "PEND", 4);\
            currently_pending_emits++;\
            state_set(SVAR(currently_pending_emits), "PEND", 4);\
        }\
}

#define CBAK_DONE(x)\
{\
    state_set(0,0, SBUF(tid)); /* remove state entry, decrement pending emitted txn counter */\
    if (currently_pending_emits - 1 > currently_pending_emits)\
        currently_pending_emits = 0;\
    else\
        currently_pending_emits--;\
    state_set(SVAR(currently_pending_emits), "PEND", 4);\
    DONE(x);\
}

#define owner_lp owner_data[0]
#define owner_fee_setting owner_data[1]

int64_t cbak(uint32_t f)
{
    uint8_t tid[32];
    otxn_id(SBUF(tid), f == 1);

    // we're going to reuse some of the template memory to avoid copying
    uint8_t* stub_ext = OTXNACC - 1;
    uint8_t* stub = stub_ext + 1;

    // remit stub consists of: 
    //  20 byte accid, 
    //  4 byte ledger seq number (that it was emitted in), 
    //  8 byte amt A XFL LE
    //  8 byte amt B XFL LE
    //  1 byte retry count (if this is the second time this emit has been tried for example)

    // stub_ext is the stub array with a P on the front, which avoids an unnecessary accid copy
    // in the event of a failed trade. we use that memory as the key to make a credit stub so the
    // emit can be retried.

    if (state(stub, 41, SBUF(tid)) != 41)
    {
        if (f == 1)
            DONE("AMM: Emit failured detected but missing state entry to link it to an account.");
        DONE("AMM: Emit callback but missing state entry to link it to an account.");
    }
    
    uint32_t currently_pending_emits = 0;
    state(SVAR(currently_pending_emits), "PEND", 4);

    int64_t owner_data[2] = { 0, 0 }; 
    
    owner_fee_setting = 6035823500676464640ULL /* 0.001 - default (0.1%), min=0 (0%), max=0.05 (5%) */;

    // this sets owner_lp and owner_fee_setting
    state(SVAR(owner_data), stub, 20);

    uint64_t sent_amt_A = *((uint64_t*)(stub + 24));
    uint64_t sent_amt_B = *((uint64_t*)(stub + 32));
    
    uint64_t A_is_zero = float_compare(sent_amt_A, 0, COMPARE_LESS | COMPARE_EQUAL) == 1;
    uint64_t B_is_zero = float_compare(sent_amt_B, 0, COMPARE_LESS | COMPARE_EQUAL) == 1;

    if (f == 0 || (A_is_zero && B_is_zero))
    {
        // the withdrawal was successful, or the amount withdrawn was 0 anyway
        
        // clean up the LAST_CUR stub if this is the last emit and it was successful
        if (currently_pending_emits == 1)
        {
            state_set(0,0, "LAST_CUR", 8);
            state_set(0,0, "LAST_FEE", 8);
        }

        CBAK_DONE("AMM: Emit callback successful.");
    }

    // execution to here means the Remit failed. We need to re-credit the user's tokens, which is a little involved
    uint8_t ammcur[80];
    int64_t already_setup = (state(SBUF(ammcur), "CUR", 3) == 80);

    // first lets check if the AMM still exists... it's possible this was the final withdrawal, in which case we need
    // to recreate the whole AMM
    if (!already_setup)
    {
        // we do this using the LAST_CUR stub left behind by the deletion routine
        if (state(SBUF(ammcur), "LAST_CUR", 8) != 80)
            NOPE("AMM: Emit failed, and AMM was deleted but no LAST_CUR stub detected. Bailing.");

        // sanity check: the final withdrawal must be a two sided withdrawal.
        if (A_is_zero || B_is_zero)
            NOPE("AMM: Cannot re-create AMM from failed one sided withdrawal.");

        // when the AMM is deleted the last fee is recorded here, so pull it back out
        state(SVAR(owner_fee_setting), "LAST_FEE", 8);

        // geometric mean constant
        int64_t G = float_multiply(sent_amt_A, sent_amt_B);

        TRACEXFL(G);
        TRACEXFL(sent_amt_A);
        TRACEXFL(sent_amt_B);
        
        // set their liquidity tokens as a state entry
        owner_lp = 6125895493223874560ULL /* 100 arbitrary liquidity units */;
        
        // compute the fee accumulator
        int64_t amm_fee_accumulator = float_multiply(owner_lp, owner_fee_setting);
        if (amm_fee_accumulator < 0)
            NOPE("AMM: Error computing initial fee accumulator (C)");
        
        if (state_set(SBUF(ammcur), "CUR", 3) != 80 ||
            state_set(SVAR(sent_amt_A), "A", 1) != 8 ||
            state_set(SVAR(sent_amt_B), "B", 1) != 8 ||
            state_set(SVAR(G), "G", 1) != 8 ||
            state_set(SVAR(amm_fee_accumulator), "FAC", 3) != 8 ||
            // remembering that owner_data is an array containing owner_lp and owner_fee_setting
            state_set(SVAR(owner_data), stub, 20) != 16 ||
            // the current total LP count is just the first member's token count
            state_set(SVAR(owner_lp), "TOT", 3) != 8)
            NOPE("AMM: Error setting initial state (reserves?) (C)");

        CBAK_DONE("AMM: AMM was recreated due to failed final emit. (Only true is this txn is tesSUCCESS.)");
    }

    // execution to here means the AMM still exists, so fetch their entry

    // either they were trading with the AMM, in which case one of the currencies will be 0
    // or they were withdrawing from the AMM in which case both will be non-zero
   
    // undo their withdrawal or trading
    int64_t amm_amt_A, amm_amt_B, G, total_lp, amm_fee_accumulator, owner_fac_contribution;
    
    // Load current AMM state
    if (!(state(SVAR(amm_amt_A), "A", 1) == 8 &&
          state(SVAR(amm_amt_B), "B", 1) == 8 &&
          state(SVAR(G), "G", 1) == 8 &&
          state(SVAR(total_lp), "TOT", 3) == 8 &&
          state(SVAR(amm_fee_accumulator), "FAC", 3) == 8))
        NOPE("AMM: Error loading hook state during emit failure reversal.");
    
    if (!A_is_zero && !B_is_zero)
    {
        // This is a withdrawal reversal
        // sent_amt_A and sent_amt_B (from stub+24 and stub+32) represent 
        // the amounts that were being withdrawn but failed to reach the user
        
        // Calculate what proportion of the pool these amounts represent in the current state
        int64_t proportion_A = float_divide(sent_amt_A, float_sum(amm_amt_A, sent_amt_A));
        int64_t proportion_B = float_divide(sent_amt_B, float_sum(amm_amt_B, sent_amt_B));
        
        // Average the proportions to get a fair representation of their ownership
        int64_t avg_proportion = float_divide(
            float_sum(proportion_A, proportion_B), 
            6090866696204910592ULL /* 2.0 */
        );
        
        // Calculate LP tokens based on current total and the proportion
        int64_t lp_to_credit = float_divide(
            float_multiply(total_lp, avg_proportion),
            float_sum(6089866696204910592ULL /* 1.0 */, float_negate(avg_proportion))
        );
        
        // Update the user's LP balance - owner_lp was already loaded from state through owner_data
        owner_lp = float_sum(owner_lp, lp_to_credit);
        
        // Update total LP in circulation
        total_lp = float_sum(total_lp, lp_to_credit);
        
        // Add the amounts back to the reserves
        amm_amt_A = float_sum(amm_amt_A, sent_amt_A);
        amm_amt_B = float_sum(amm_amt_B, sent_amt_B);
        
        // Recompute geometric mean constant
        G = float_multiply(amm_amt_A, amm_amt_B);
        
        // Update fee accumulator with the user's contribution
        // The fee setting was already loaded at the beginning of cbak function
        owner_fac_contribution = float_multiply(owner_lp, owner_fee_setting);
        amm_fee_accumulator = float_sum(amm_fee_accumulator, owner_fac_contribution);
        
        TRACESTR("AMM: Reversing withdrawal");
        TRACEXFL(sent_amt_A);
        TRACEXFL(sent_amt_B);
        TRACEXFL(lp_to_credit);
    
        // Validate results
        if (amm_amt_A <= 0 || amm_amt_B <= 0 || total_lp < 0)
            NOPE("AMM: Error computing values during failed operation recovery.");
        
        // Recompute geometric mean constant
        G = float_multiply(amm_amt_A, amm_amt_B);
    
        // Update the AMM state with restored values
        if (state_set(SVAR(amm_amt_A), "A", 1) != 8 ||
            state_set(SVAR(amm_amt_B), "B", 1) != 8 ||
            state_set(SVAR(G), "G", 1) != 8 ||
            state_set(SVAR(total_lp), "TOT", 3) != 8 ||
            state_set(SVAR(amm_fee_accumulator), "FAC", 3) != 8 ||
            state_set(SVAR(owner_data), stub, 20) != 16)
            NOPE("AMM: Error restoring AMM state after failed operation.");
    
        CBAK_DONE("AMM: Restored state after failed withdrawal operation.");
    }

    
    // Execution to here means it was a failed trade    
    // Trading can't be reversed because the user is transient and has no "account" with the
    // hook to credit. So rather than trying to reverse it and send back out the original amount
    // we'll create a credit stub for them, that they can fetch with an empty remit.

    // credit stub is:
    //  8 byte XFL LE amm_amt_A, 
    //  8 byte XFL LE amm_amt_B,
    //  1 byte retry count,
    //  and is's keyed on 0x14, <accid> (21 bytes)
    // note that a credit stub is different to an emit stub
    // the emit stub is created when the hook emits a txn and is keyed against the emitted txn id
    // the credit stub is created when the emitted txn failed and it was for a trade, so the AMM
    // owes the user a retry of the emit. The credit stub is keyed with a 0x14 followed by the 20 byte acc id.
    
    if (stub[40]++ > MAX_EMIT_RETRIES)
    {
        // retry count is too high, they forfeit
        CBAK_DONE("AMM: You emit failed for the final possible time. Credit is forfeit.");
    }

    state_set(stub + 24, 17, stub_ext, 21);
    CBAK_DONE("AMM: Your emit failed. Send empty Remit to try again.");
}

int64_t hook(uint32_t r)
{
    etxn_reserve(1);
    _g(1,1);

    TRACESTR("AMM starting");

    int64_t tt = otxn_type();

    hook_account(HOOKACC, 20);

    otxn_field(OTXNACC, 20, sfAccount);

    if (BUFFER_EQUAL_20(OTXNACC, HOOKACC))
        DONE("AMM: Passing outgoing txn.");
    
    if (tt != ttREMIT)
        DONE("AMM: Passing non-REMIT txn.");

    // check account xah balance and refuse to do anything if it's not met
    {
        uint8_t keylet[34];
        int64_t bal_xfl = 0;
        if (util_keylet(keylet, 34, KEYLET_ACCOUNT, OTXNACC, 20, 0,0,0,0) != 34 || 
            slot_set(keylet, 34, 99) != 99 ||
            slot_subfield(99, sfBalance, 99) != 99 ||
            (bal_xfl = slot_float(99)) < 0)
            NOPE("AMM: Internal error accroot slot/keylet");

        if (float_compare(bal_xfl, 6107881094714392576ULL /* 10.0 XAH */, COMPARE_LESS))
           NOPE("AMM: Insufficient XAH balance in hook account. Top up using a ttPayment to continue using.");
    } 


    // first ensure they have not transferred any URITokens with the remit
    // because this is an attack vector, can use up all the directory space in the AMM
    
    otxn_slot(1);

    if (slot_subfield(1, sfMintURIToken, 2) == 2 || slot_subfield(1, sfURITokenIDs, 2) == 2)
        NOPE("AMM: Cannot accept REMIT with URITokens.");
    
    // check how many currencies were sent
    int64_t sent_currency_count = 
        slot_subfield(1, sfAmounts, 2) == 2
        ?  slot_count(2)
        : 0;

    // valid numbers of currencies are 0, 1 and 2, depending on what's happening, more than 2 is always an error
    if (sent_currency_count < 0 || sent_currency_count > 2)
        NOPE("AMM: Send either 0 (withdraw), 1 (trade) or 2 (deposit) currencies to use AMM.");
    
    // if the AMM is already setup grab the currency information for it 
    #define amm_cur_A ammcur
    #define amm_cur_B (ammcur + 40)
    uint8_t ammcur[80];
    int64_t already_setup = (state(SBUF(ammcur), "CUR", 3) == 80);

    int64_t A_is_xah = ISZERO20(amm_cur_A);
    int64_t B_is_xah = ISZERO20(amm_cur_B);

  
    // Check if this user has a credit stub (from a previous failed trade remit)
    // if they do then the AMM is blocked to them until either the credit is claimed or forfeit
    // We need to query the user's account with a 0x14 at the first byte, but happily that's how
    // it appears in the template memory already so we just need to back the pointer up by one
    uint8_t credit_stub[17];
    if (state(SBUF(credit_stub), OTXNACC-1, 21) == 17)
    {
        // a credit stub exists for this user
        if (sent_currency_count > 0)
            NOPE("AMM: You have pending funds to claim. Send an empty REMIT transaction first. "
                "Use SKIPCREDIT parameter on empty REMIT to cancel after next attempt if stuck.");

        // check if they opt to forfeit the credit on failed retry
        uint8_t retry_count = credit_stub[16];
        uint8_t skip_credit = 0;
        if (otxn_param(SVAR(skip_credit), "SKIPCREDIT", 10) == 1 && skip_credit != 0)
            credit_stub[16] = MAX_EMIT_RETRIES + 1; // this will force next retry to be the last

        uint64_t owed_amt_A = *((uint64_t*)(credit_stub));
        uint64_t owed_amt_B = *((uint64_t*)(credit_stub + 8));
        
        // Check if either amount is non-zero
        uint64_t A_is_zero = float_compare(owed_amt_A, 0, COMPARE_LESS | COMPARE_EQUAL) == 1;
        uint64_t B_is_zero = float_compare(owed_amt_B, 0, COMPARE_LESS | COMPARE_EQUAL) == 1;
        
        if (!already_setup)
        {
            // RH NOTE: This should never happen, but if it does remove the stub and pass the txn
            state_set(0,0, OTXNACC-1, 21);
            DONE("AMM: Cannot process accounts payable as AMM no longer exists.");
        }
        
        int64_t A_is_xah = ISZERO20(amm_cur_A);
        int64_t B_is_xah = ISZERO20(amm_cur_B);
        
        // Create remit with the owed amounts
        if (!A_is_zero) 
        {
            // They were trading for currency A
            if (A_is_xah)
                float_sto(TXN_CUR_A + 1, 8, 0, 0, 0, 0, XAH_TO_DROPS(owed_amt_A), 0);
            else
            {
                ENSURE_TRUSTLINE_EXISTS(ammcur + 0, ammcur + 20);
                float_sto(TXN_CUR_A, 49, ammcur + 0, 20, ammcur + 20, 20, owed_amt_A, sfAmount);
            }
        }
        
        if (!B_is_zero)
        {
            // They were trading for currency B
            if (!A_is_zero)
            {
                // Two-sided payable - unlikely but possible in case both sides were being paid out
                if (B_is_xah)
                    float_sto(TXN_CUR_B + 1, 8, 0, 0, 0, 0, XAH_TO_DROPS(owed_amt_B), 0);
                else
                {
                    ENSURE_TRUSTLINE_EXISTS(ammcur + 40, ammcur + 60);
                    float_sto(TXN_CUR_B, 49, ammcur + 40, 20, ammcur + 60, 20, owed_amt_B, sfAmount);
                }
                
                // Emit both currencies
                DO_REMIT(0, owed_amt_A, owed_amt_B, retry_count);
            }
            else
            {
                // Only B is owed
                if (B_is_xah)
                    float_sto(TXN_CUR_A + 1, 8, 0, 0, 0, 0, XAH_TO_DROPS(owed_amt_B), 0);
                else
                {
                    ENSURE_TRUSTLINE_EXISTS(ammcur + 40, ammcur + 60);
                    float_sto(TXN_CUR_A, 49, ammcur + 40, 20, ammcur + 60, 20, owed_amt_B, sfAmount);
                }
                
                // Emit only one currency
                DO_REMIT(1, owed_amt_B, 0, retry_count);
            }
        }
        else
        {
            // Only A is owed
            DO_REMIT(1, owed_amt_A, 0, retry_count);
        }
        
        // remove the stub since we've emitted a txn
        state_set(0,0, OTXNACC-1, 21);
        DONE("AMM: Emitted credit for previously failed emit.");
    }


    // grab the amounts, constant and current outstanding liquidity points
    int64_t amm_amt_A, amm_amt_B, G, total_lp;
  
    // grab proposed fee if any (for fee voting or for setup)... use a default if not specified.
    uint64_t otxn_fee_preference = 6035823500676464640ULL /* 0.001 - default (0.1%), min=0 (0%), max=0.05 (5%) */;
    int64_t otxn_has_fee = otxn_param(SVAR(otxn_fee_preference), "FEE", 3) > 0;

    if (float_compare(otxn_fee_preference, MAX_AMM_FEE /* 5% */, COMPARE_GREATER) == 1 ||
        float_compare(otxn_fee_preference, 0, COMPARE_LESS) == 1)
        NOPE("AMM: Invalid proposed FEE, must be between 0 and 0.05 (5%) as XFL LE");

    // owner data is packed int64_t LE sent and retrieved in memory format from hook state
    int64_t owner_data[2] = { 0, 0 }; 
    //#define owner_lp owner_data[0]
    //#define owner_fee_setting owner_data[1]

    // grab the user's liquidity tokens, and fee setting, if these are set
    // this sets owner_lp and owner_fee_setting
    state(SVAR(owner_data), OTXNACC, 20);

    uint64_t seq = ledger_seq() + 1;

    int64_t amm_fee_accumulator = 0;

    // compute current fee accumulator contribution, which will be 0 if unset
    int64_t owner_fac_contribution = float_multiply(owner_lp, owner_fee_setting);
  
    if (already_setup)
    {
        // load and sanity check values
        if (!(state(SVAR(amm_amt_A), "A", 1) == 8 &&
            state(SVAR(amm_amt_B), "B", 1) == 8 &&
            state(SVAR(G), "G", 1) == 8 &&
            state(SVAR(total_lp), "TOT", 3) == 8 &&
            state(SVAR(amm_fee_accumulator), "FAC", 3) == 8))
            NOPE("AMM: Error loading hook state.");
    }

    int64_t amm_fee = float_divide(amm_fee_accumulator, total_lp);

    // clamp the fee to the allowable range
    if (float_compare(amm_fee, 0, COMPARE_LESS) == 1 || amm_fee < 0)
        amm_fee = 0;
    else if (float_compare(amm_fee, MAX_AMM_FEE, COMPARE_GREATER) == 1)
        amm_fee = MAX_AMM_FEE;

    // invert the fee so that it can be multiplied against amounts going out
    amm_fee = float_sum(6089866696204910592ULL /* 1.000 */, float_negate(amm_fee));

    // if for some reason this is an insane value (should never happen) set it to parity
    // this is because it's better to allow the AMM to continue with no fee in an error state
    // than to brick it and prevent withdrawal or use
    if (amm_fee < 0 || 
            float_compare(amm_fee, 6089866696204910592ULL, COMPARE_GREATER) == 1 ||
            float_compare(amm_fee, 0, COMPARE_LESS) == 1)
        amm_fee = 1;


    uint8_t isu[20];
    uint8_t cur[20];


    // First operation we'll deal with is a withdrawal. This happens if they send an empty remit.
    // All of their LP tokens are converted to currency and remitted back to them, unless WDR is specified
    // containing the number of LP tokens to liquidate, in which case a partial withdrawal is actioned.
    if (sent_currency_count == 0)
    {
        // can't withdraw unless it's already created
        if (!already_setup)
            NOPE("AMM: Setup the AMM by remitting two currencies.");

        // can't withdraw if you don't have tokens
        if (!float_compare(owner_lp, 0, COMPARE_GREATER))
            NOPE("AMM: You don't have any liquidity tokens in this pool.");

        // if they are more than total somehow, just assume they own all of them
        if (float_compare(owner_lp, total_lp, COMPARE_GREATER))
            owner_lp = total_lp;
        
        // check if this is a partial withdrawal, meaning they send a parameter of type "WDR"
        // which is an XFL LE containing the number of LP tokens they are withdrawing.

        uint64_t withdraw_lp = owner_lp;
        uint64_t new_owner_lp = 0;

        otxn_param(SVAR(withdraw_lp), "WDR", 3);

        if (withdraw_lp <= 0 || float_compare(withdraw_lp, 0, COMPARE_LESS | COMPARE_EQUAL) == 1)
            NOPE("AMM: Invalid WDR parameter, specify a positive number of LP tokens to liquidate.");

        int64_t withdrawal_percent = 6089866696204910592ULL /* 1.00 */;
        int64_t remain_percent = 0;
        if (float_compare(withdraw_lp, owner_lp, COMPARE_GREATER | COMPARE_EQUAL) == 1)
            withdraw_lp = owner_lp;
        else
        {
            withdrawal_percent = float_divide(withdraw_lp, owner_lp);
            remain_percent = float_sum(6089866696204910592ULL /* 1.00 */, float_negate(withdrawal_percent));

            // sanity check for negatives
            if (float_compare(withdrawal_percent, 0, COMPARE_LESS | COMPARE_EQUAL) == 1 ||
                float_compare(remain_percent, 0, COMPARE_LESS | COMPARE_EQUAL) == 1)
                NOPE("AMM: Error computing withdrawal percent.");

            int64_t sum = float_sum(withdrawal_percent, remain_percent);

            // sanity check percentage calculations
            if (float_compare(withdrawal_percent, 6089866696204910592ULL /* 1 */, COMPARE_GREATER) == 1 ||
                float_compare(remain_percent, 6089866696204910592ULL /* 1 */, COMPARE_GREATER) == 1 ||
                float_compare(sum, 6089866696204910592ULL /* 1.00000000000 */, COMPARE_EQUAL) != 1)
                NOPE("AMM: Error sanity checking withdrawal percent.");

            new_owner_lp = float_sum(owner_lp, float_negate(withdraw_lp));
            if (float_compare(new_owner_lp, 0, COMPARE_LESS) == 1)
                new_owner_lp = 0;
        }

        if (withdrawal_percent <= 0 || remain_percent < 0)
            NOPE("AMM: Error computing withdraw/remain %");

        // enforce minimum withdrawal percent of 1% to help avoid spam
        if (float_compare(withdrawal_percent, 6053837899185946624ULL /* 0.01  (1%) */, COMPARE_LESS) == 1)
            NOPE("AMM: Minimum withdrawal amount is 1% of holdings.");

        // enforce maximum partial withdrawal percent of 99% to help avoid dust
        if (float_compare(withdrawal_percent, 6080752297695428608ULL /* 0.99 (99%) */, COMPARE_GREATER) == 1 &&
            remain_percent != 0)
            NOPE("AMM: To withdraw all omit the WDR param on the REMIT txn.");

        // update fee accumulator to take into account the withdrawal
        amm_fee_accumulator = float_sum(amm_fee_accumulator,
                float_negate(owner_fac_contribution));

        // if it's a partial withdrawal then replenish their FAC contribution based on remaining tokens
        // and optionally also modified FEE vote during this transaction
        if (remain_percent != 0)
        {
            if (otxn_has_fee)
                owner_fee_setting = otxn_fee_preference;

            owner_fac_contribution = float_multiply(new_owner_lp, owner_fee_setting);
            amm_fee_accumulator = float_sum(amm_fee_accumulator, owner_fac_contribution);
        }

        // if somehow through rounding the accumulator ends up under 0 set it to 0
        if (float_compare(amm_fee_accumulator, 0, COMPARE_LESS) == 1)
            amm_fee_accumulator = 0;

        // pre-withdrawal ownership percentage
        int64_t ownership_percent = float_divide(owner_lp, total_lp);
        
        if (ownership_percent < 0)
            NOPE("AMM: Error computing ownership %");
        
        int64_t out_amt_A = 
            float_multiply(
                float_multiply(amm_amt_A, ownership_percent),
                withdrawal_percent /* will be 1.00 for complete withdrawal */);
        int64_t out_amt_B =
            float_multiply(
                float_multiply(amm_amt_B, ownership_percent),
                withdrawal_percent);

        // clamp the amounts we're sending out
        int64_t send_all_A = 0, send_all_B = 0;
        if (!float_compare(out_amt_A, amm_amt_A, COMPARE_LESS))
        {
            out_amt_A = amm_amt_A;
            send_all_A = 1;
        }
        if (!float_compare(out_amt_B, amm_amt_B, COMPARE_LESS))
        {
            out_amt_B = amm_amt_B;
            send_all_B = 1;
        }

        total_lp = float_sum(total_lp, float_negate(withdraw_lp));
        if (total_lp < 0)
            NOPE("AMM: Error computing total_lp");

        int64_t is_final_withdrawal = (float_compare(total_lp, 0, COMPARE_LESS | COMPARE_EQUAL) == 1);

        uint32_t currently_pending_emits = 0;
        state(SVAR(currently_pending_emits), "PEND", 4);

        // prevent a nasty race condition where the AMM is re-created in cbak using a failed one-sided remit
        if (currently_pending_emits > 0 && is_final_withdrawal)
            NOPE("AMM: The final withdrawal from an AMM pool can only be made after "
                    "all other pending withdrawals are complete. Try again in a bit.");

        // compute the new values
        amm_amt_A = send_all_A ? 0 : float_sum(amm_amt_A, float_negate(out_amt_A));
        amm_amt_B = send_all_B ? 0 : float_sum(amm_amt_B, float_negate(out_amt_B));

        // delete owner's entry
        if (remain_percent == 0)
            state_set(0,0, OTXNACC, 20);
            
        if (is_final_withdrawal)
        {
            // this is the final withdrawal, so remove setup information

            // delete all entries
            state_set(0,0, "TOT", 3);
            state_set(0,0, "A", 1);
            state_set(0,0, "B", 1);
            state_set(0,0, "CUR", 3);
            state_set(0,0, "G", 1);
            state_set(0,0, "FAC", 3);

            // leave behind a stub indicating which currency pair was used
            // at the time of deletion in case the emit fails and we need to
            // rebuild the amm we'll clean this up at the callback if the emit
            // is successful
            state_set(SBUF(ammcur), "LAST_CUR", 8);
            state_set(SVAR(owner_fee_setting), "LAST_FEE", 8);
        }
        else
        { 
            // compute new G
            int64_t new_G = float_multiply(amm_amt_A, amm_amt_B);
            if (new_G <= 0)
                NOPE("AMM: Internal error when calculating new G.");
        
            owner_lp = new_owner_lp;

            if (remain_percent != 0 && state_set(SVAR(owner_data), OTXNACC, 20) != 16)
                NOPE("AMM: Error updating user during withdrawal (reserves?)");

            if (state_set(SVAR(total_lp), "TOT", 3) != 8 ||
                state_set(SVAR(amm_amt_A), "A", 1) != 8 ||
                state_set(SVAR(amm_amt_B), "B", 1) != 8 ||
                state_set(SVAR(new_G), "G", 1) != 8 ||
                state_set(SVAR(amm_fee_accumulator), "FAC", 3) != 8)
                NOPE("AMM: Error updating parameters during withdrawal (reserves?)");
        }

        // write amounts into remit
        if (A_is_xah)
            float_sto(TXN_CUR_A + 1, 8, 0,0,0,0, XAH_TO_DROPS(out_amt_A), 0);
        else
        {
            ENSURE_TRUSTLINE_EXISTS(ammcur + 0, ammcur + 20);
            float_sto(TXN_CUR_A, 49, ammcur +  0, 20, ammcur + 20, 20, out_amt_A, sfAmount);
        }

        if (B_is_xah)
            float_sto(TXN_CUR_B + 1, 8, 0,0,0,0, XAH_TO_DROPS(out_amt_B), 0);
        else
        {
            ENSURE_TRUSTLINE_EXISTS(ammcur + 40, ammcur + 60);
            float_sto(TXN_CUR_B, 49, ammcur + 40, 20, ammcur + 60, 20, out_amt_B, sfAmount);
        }

        DO_REMIT(0, out_amt_A, out_amt_B, 0);


        DONE("AMM: Emitted withdraw.");
        return 0;
    }
    
    // check the execution hasn't dropped through somehow from above
    if (sent_currency_count == 0)
        NOPE("AMM: Internal error (1).");

    // execution to here means we're either using the pool or depositing to the pool

    uint8_t sent_cur_A[49];
    uint8_t sent_cur_B[49];

    if (slot_subarray(2, 0, 3) != 3)
        NOPE("AMM: Error slotting currency A");

    int64_t has_sent_B = (slot_subarray(2, 1, 4) == 4);

    slot_subfield(3, sfAmount, 3);
    slot_subfield(4, sfAmount, 4);

    int64_t sent_xah_A = (slot_type(3, 1) == 1);
    int64_t sent_xah_B = (slot_type(4, 1) == 1);

    slot(SBUF(sent_cur_A), 3);

    // if sent_cur_B doesn't exist, this fails but it isn't an issue
    slot(SBUF(sent_cur_B), 4);

    int64_t sent_amt_A = slot_float(3);
    int64_t sent_amt_B = slot_float(4);

    // should be impossible, check anyway
    if (has_sent_B && BUFFER_EQUAL_40(sent_cur_A+1, sent_cur_B+1))
        NOPE("AMM: Both currencies cannot be the same.");

    // sanity check amount reads
    if (sent_amt_A < 0 || 
        (has_sent_B && sent_amt_B < 0))
            NOPE("AMM: Error reading sent amounts.");

    // check for trivial/dust amounts
    // we can do this by dividing the amount by 10000 and seeing if it undeflows to 0
    int64_t sent_amt_A_test = float_divide(sent_amt_A, 6161924290242838528ULL /* 10000 */);
    int64_t sent_amt_B_test = float_divide(sent_amt_B, 6161924290242838528ULL /* 10000 */);
    if (float_compare(sent_amt_A_test, 0, COMPARE_LESS | COMPARE_EQUAL) == 1 ||
        (has_sent_B && float_compare(sent_amt_B_test, 0, COMPARE_LESS | COMPARE_EQUAL) == 1))
        NOPE("AMM: Amounts must be positive, non-dust, non-zero.");

    // check if the AMM has been setup yet
    if (!already_setup)
    {
        // not setup, so whatever they sent is the first liquidity and sets the rate
        if (!has_sent_B)
            NOPE("AMM: Cannot setup new AMM without two currencies. Send two.");

        uint8_t dummy[80];
        if (state(SVAR(dummy), "LAST_CUR", 8) == 80)
            NOPE("AMM: Cannot setup new AMM until final withdrawal is finalized from previous AMM.");

        // the format for the packed data is
        // [ 20 byte currency code A ][ 20 byte issuer A or 0's for XAH ]
        // [ 20 byte currency code B ][ 20 byte issuer B or 0's for XAH ]
        
        // copy currency A into the array
        if (!sent_xah_A)
            COPY40(sent_cur_A + 8, ammcur);

        if (!sent_xah_B)
            COPY40(sent_cur_B + 8, ammcur + 40);

        // geometric mean constant
        int64_t G = float_multiply(sent_amt_A, sent_amt_B);

        TRACEXFL(G);
        TRACEXFL(sent_amt_A);
        TRACEXFL(sent_amt_B);
        
        // set their liquidity tokens as a state entry
        owner_lp = 6125895493223874560ULL /* 100 arbitrary liquidity units */;

        // set their fee preference
        owner_fee_setting = otxn_fee_preference;
        
        // compute the fee accumulator
        amm_fee_accumulator = float_multiply(owner_lp, owner_fee_setting);
        if (amm_fee_accumulator < 0)
            NOPE("AMM: Error computing initial fee accumulator");
        
        if (state_set(SBUF(ammcur), "CUR", 3) != 80 ||
            state_set(SVAR(sent_amt_A), "A", 1) != 8 ||
            state_set(SVAR(sent_amt_B), "B", 1) != 8 ||
            state_set(SVAR(G), "G", 1) != 8 ||
            state_set(SVAR(amm_fee_accumulator), "FAC", 3) != 8 ||
            // remembering that owner_data is an array containing owner_lp and owner_fee_setting
            state_set(SVAR(owner_data), OTXNACC, 20) != 16 ||
            // the current total LP count is just the first member's token count
            state_set(SVAR(owner_lp), "TOT", 3) != 8)
            NOPE("AMM: Error setting initial state (reserves?)");

        DONE("AMM: Created (only if this tx has a tesSUCCESS code.)");
    }

    // trace(SBUF("sent_cur_A"), sent_cur_A, 40, 1);
    // trace(SBUF("amm_cur_A "), amm_cur_A, 40, 1);
    // trace(SBUF("sent_cur_B"), sent_cur_B, 40, 1);
    // trace(SBUF("amm_cur_B "), amm_cur_B, 40, 1);

    // find out which sent currencies map to which currencies in the AMM, if any
    int64_t mapA = -1;
    if (BUFFER_EQUAL_40(sent_cur_A + 8, amm_cur_A))
        mapA = 0;
    else if (BUFFER_EQUAL_40(sent_cur_A + 8, amm_cur_B))
        mapA = 1;


    int64_t mapB = -1;
    if (BUFFER_EQUAL_40(sent_cur_B + 8, amm_cur_A))
        mapB = 0;
    else if (BUFFER_EQUAL_40(sent_cur_B + 8, amm_cur_B))
        mapB = 1;


    if (mapA == -1 || (mapB == -1 && has_sent_B))
        NOPE("AMM: Tried to send in currency which is not present in AMM.");

    if (mapA == mapB)
        NOPE("AMM: Internal error (2).");

    int64_t has_sent_A = 1;

    // flip if they came in backwards
    if (mapA == 1)
    {
        int64_t c = sent_amt_A;
        sent_amt_A = sent_amt_B;
        sent_amt_B = c;

        if (!has_sent_B)
        {
            // if they sent only one currency and it maps to amm_B,
            // then we need to mark that
            has_sent_A = 0;
            has_sent_B = 1;
        }
    }

    // execution to here means the A and B variables in the sent namespace match those in the amm namespace.
    if (has_sent_A && has_sent_B)
    {
        // first scenario: they sent both currencies, this is a deposit

        // remove their previous contribution to the AMM fee accumulator
        amm_fee_accumulator = float_sum(amm_fee_accumulator, float_negate(owner_fac_contribution));
        int64_t new_amt_A = float_sum(amm_amt_A, sent_amt_A);
        int64_t new_amt_B = float_sum(amm_amt_B, sent_amt_B);

        int64_t amm_ratio = float_divide(amm_amt_A, amm_amt_B);
        int64_t sent_ratio = float_divide(sent_amt_A, sent_amt_B);

        int64_t diverge = float_divide(amm_ratio, sent_ratio);

        // sanity check float operations, keeping in mind a negative result from float functions means error
        // however 0 xfl is 0 decimal
        if (amm_ratio <= 0 || sent_ratio <=0 || diverge < 0)
            NOPE("AMM: Error computing amm ratio.");
        
        // allowable values 0.99 - 1.01
        if (float_compare(diverge, 6080752297695428608ULL /* 0.99 ( 99%) */, COMPARE_LESS) == 1 ||
            float_compare(diverge, 6089876696204910592ULL /* 1.01 (101%) */, COMPARE_GREATER) == 1)
            NOPE("AMM: Divergence too great. Send amounts at the correct ratio.");

        int64_t new_G = float_multiply(new_amt_A, new_amt_B);

        // work out how many LP tokens they get
        // we'll work out their percentage ownership of A and B and average these
        
        int64_t ownership_A = float_divide(sent_amt_A, amm_amt_A);
        int64_t ownership_B = float_divide(sent_amt_B, amm_amt_B);

        if (ownership_A <= 0 || ownership_B <= 0)
            NOPE("AMM: Internal error (3).");

        int64_t ownership = float_divide(float_sum(ownership_A, ownership_B), 6090866696204910592ULL /* 2 */);

        int64_t new_lp = float_multiply(ownership, total_lp);

        if (float_compare(float_sum(total_lp, new_lp), total_lp, COMPARE_LESS) != 0 ||
            float_compare(float_sum(owner_lp, new_lp), owner_lp, COMPARE_LESS) != 0)
            NOPE("AMM: Overflow error.");

        total_lp = float_sum(total_lp, new_lp);
        owner_lp = float_sum(owner_lp, new_lp);

        amm_amt_A = float_sum(amm_amt_A, sent_amt_A);
        amm_amt_B = float_sum(amm_amt_B, sent_amt_B);

        if (amm_amt_A <= 0 || amm_amt_B <= 0 || total_lp <= 0 || owner_lp <= 0)
            NOPE("AMM: Error crediting new balances to internal AMM state.");
        
        // set their fee preference
        owner_fee_setting = otxn_fee_preference;
        
        // compute the fee accumulator
        owner_fac_contribution = float_multiply(owner_lp, owner_fee_setting);

        amm_fee_accumulator = float_sum(amm_fee_accumulator, owner_fac_contribution);

        if (amm_fee_accumulator < 0)
            NOPE("AMM: Error computing initial fee accumulator");
        
        // ok credit their liquidity tokens
        if (state_set(SVAR(owner_data), OTXNACC, 20) != 16 ||
            state_set(SVAR(total_lp), "TOT", 3) != 8 ||
            state_set(SVAR(amm_amt_A), "A", 1) != 8 ||
            state_set(SVAR(amm_amt_B), "B", 1) != 8 ||
            state_set(SVAR(new_G), "G", 1) != 8 ||
            state_set(SVAR(amm_fee_accumulator), "FAC", 3) != 8)
            NOPE("AMM: Error crediting liquidity (reserves?)");

        DONE("AMM: Liquidity added.");
    }

    // execution to here means they are not adding liquidity, but rather using the pool

    if (float_compare(amm_amt_A, 0, COMPARE_EQUAL | COMPARE_LESS) == 1 ||
        float_compare(amm_amt_B, 0, COMPARE_EQUAL | COMPARE_LESS) == 1)
        NOPE("AMM: Invariant failure - A or B <= 0.");

    if (has_sent_A)
    {
        // sent only A, so return only B

        int64_t new_amt_A = float_sum(amm_amt_A, sent_amt_A);

        int64_t calc_amt_B = float_divide(G, new_amt_A);

        int64_t diff_B = float_sum(amm_amt_B, float_negate(calc_amt_B));

        // apply fee
        diff_B = float_multiply(amm_fee, diff_B);

        if (diff_B <= 0 || !float_compare(diff_B, 0, COMPARE_GREATER))
            NOPE("AMM: Error computing currency B.");

        TRACEXFL(amm_amt_B);
        
        // subtract
        amm_amt_B = float_sum(amm_amt_B, float_negate(diff_B));

        if (amm_amt_B <= 0)
            NOPE("AMM: Error computing currency B.");

        TRACEXFL(amm_amt_A);
        TRACEXFL(amm_amt_B);

        // invariant
        if (float_compare(float_multiply(new_amt_A, amm_amt_B), G, COMPARE_GREATER | COMPARE_EQUAL) != 1)
            NOPE("AMM: Invariant failure A*B<G.");

        state_set(SVAR(new_amt_A), "A", 1);
        state_set(SVAR(amm_amt_B), "B", 1);

        // write amount into remit (it's written to spot A in the out array, but it's currency B)
        if (B_is_xah)
            float_sto(TXN_CUR_A + 1, 8, 0, 0, 0, 0, XAH_TO_DROPS(diff_B), 0); // XAH
        else
        {
            ENSURE_TRUSTLINE_EXISTS(ammcur + 40, ammcur + 60);
            float_sto(TXN_CUR_A, 49, ammcur +  40, 20, ammcur + 60, 20, diff_B, sfAmount);
        }
        
        DO_REMIT(1, diff_B, 0, 0);
    }
    else
    {
        int64_t new_amt_B = float_sum(amm_amt_B, sent_amt_B);

        int64_t calc_amt_A = float_divide(G, new_amt_B);

        int64_t diff_A = float_sum(amm_amt_A, float_negate(calc_amt_A));
        
        // apply fee
        diff_A = float_multiply(amm_fee, diff_A);

        TRACEXFL(new_amt_B);
        TRACEXFL(calc_amt_A);
        TRACEXFL(diff_A);
        if (diff_A <= 0 || !float_compare(diff_A, 0, COMPARE_GREATER))
            NOPE("AMM: Error computing currency A.");

        TRACEXFL(amm_amt_A);
        
        // subtract
        amm_amt_A = float_sum(amm_amt_A, float_negate(diff_A));

        if (amm_amt_A <= 0)
            NOPE("AMM: Error computing currency A.");

        TRACEXFL(amm_amt_A);
        TRACEXFL(amm_amt_B);
        
        // invariant
        if (float_compare(float_multiply(amm_amt_A, new_amt_B), G, COMPARE_GREATER | COMPARE_EQUAL) != 1)
            NOPE("AMM: Invariant failure A*B<G.");

        state_set(SVAR(amm_amt_A), "A", 1);
        state_set(SVAR(new_amt_B), "B", 1);

        // write amount into remit depending on if it's XAH or not
        if (A_is_xah)
            float_sto(TXN_CUR_A + 1, 8, 0, 0, 0, 0, XAH_TO_DROPS(diff_A), 0);
        else
        {
            ENSURE_TRUSTLINE_EXISTS(ammcur + 0, ammcur + 20);
            float_sto(TXN_CUR_A, 49, ammcur +  0, 20, ammcur + 20, 20, diff_A, sfAmount);
        }
    
        DO_REMIT(1, diff_A, 0, 0);
    }

    DONE("AMM: Emitted remit currency B.");
    return 0;
}

