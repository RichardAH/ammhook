#include "hookapi.h"
#define ttREMIT 95
#define DONE(x)\
    return accept(x, sizeof(x), __LINE__);
#define NOPE(x)\
    return rollback(x, sizeof(x), __LINE__);

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
/* 116, 113 */   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,    /* emit detail */
                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,

/*   2, 229 */  0xF0U, 0x5CU,                                                               /* lead-in amount array */
/*   2, 231 */  0xE0U, 0x5BU,                                                               /*lead-in amount entry A*/
/*  49, 233 */  0x61U,
                0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                            /* amount A */
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*   3, 282 */  0xE1, 0xE0U, 0x5BU,                                                         /*lead-in amount entry B*/
/*  49, 285 */  0x61U,
                0,0,0,0,0,0,0,0,
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,                                            /* amount B */
                0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
/*   2, 334 */  0xE1, 0xF1                                                 /* lead out, may also appear at end of A */
/*   -, 336 */                
};

#define TXN_CUR_A (txn_remit + 233)
#define TXN_CUR_B (txn_remit + 284)
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

#define DO_REMIT(early)\
{\
        int64_t bytes = 336;\
        if (early)\
        {\
            txn_remit[283] = 0xF1U;\
            bytes = 284;\
        }\
        etxn_details(TXN_EDET, 116);\
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
}

uint8_t query_out[64] = {
/* size,upto */   
/*   8,   0  */ 'A', 'M', 'M', ' ', 'P', 'B', 'D', 0,
/*  20,   8  */  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0,  // currency
/*  20,  28  */  0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0,  0,0,0,0,  // issuer
/*   8,  48  */  0,0,0,0, 0,0,0,0,                              // xfl amt
/*   8,  56  */  0,0,0,0, 0,0,0,0                               // xfl exchange rate
/*   -,  64  */
};

#define QOUT_ISU (query_out + 28)
#define QOUT_CUR (query_out + 8)
#define QOUT_AMT (*(uint64_t*)(query_out + 48))
#define QOUT_XCH (*(uint64_t*)(query_out + 56))
#define QOUT_SIZE 64

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
    
    if (tt != ttREMIT && tt != ttINVOKE)
        DONE("AMM: Passing non-REMIT non-INVOKE txn.");
   
    // if the AMM is already setup grab the currency information for it 
    #define amm_cur_A ammcur
    #define amm_cur_B (ammcur + 40)
    uint8_t ammcur[80];
    int64_t already_setup = (state(SBUF(ammcur), "CUR", 3) == 80);

    // grab the amounts, constant and current outstanding liquidity points
    int64_t amm_amt_A, amm_amt_B, G, owner_lp, total_lp;
  
    if (already_setup && !(state(SVAR(amm_amt_A), "A", 1) == 8 &&
        state(SVAR(amm_amt_B), "B", 1) == 8 &&
        state(SVAR(G), "G", 1) == 8 &&
        state(SVAR(total_lp), "TOT", 3) == 8))
        NOPE("AMM: Error loading hook state.");
    
    int64_t is_query = (tt == ttINVOKE);
    uint8_t isu[20];
    uint8_t cur[20];
    int64_t query_amt;

    // query interface XAS-1
    if (is_query)
    {

        if (!already_setup)
            NOPE("AMM: Querying AMM that has not yet been setup. Remit two currencies to setup.");
        if (hook_param(SBUF(isu), "ISU", 3) != 20)
            NOPE("AMM: Query lacked 20 byte ISU (Issuer) parameter.");
        if (hook_param(SBUF(cur), "CUR", 3) != 20)
            NOPE("AMM: Query lacked 20 byte CUR (Currency) parameter.");
        if (hook_param(SVAR(query_amt), "AMT", 3) != 8)
            NOPE("AMM: Query lacked 8 byte XFL AMT (Amount) parameter.");
        if (query_amt <= 0 || !float_compare(query_amt, 0, COMPARE_GREATER))
            NOPE("AMM: Query invalid amount. Try a positive LE XFL.");


        if (BUFFER_EQUAL_20(cur, amm_cur_A) && BUFFER_EQUAL_20(isu, amm_cur_A + 20))
        {
            // currency A
            int64_t new_amt_A = float_sum(amm_amt_A, query_amt);
            int64_t calc_amt_B = float_divide(G, new_amt_A);
            int64_t diff_B = float_sum(amm_amt_B, float_negate(calc_amt_B));

            if (diff_B <= 0 || !float_compare(diff_B, 0, COMPARE_GREATER))
                NOPE("AMM: Error computing currency B.");

            COPY20(amm_cur_B, QOUT_CUR);
            COPY20(amm_cur_B + 20, QOUT_ISU);
            
            QOUT_AMT = diff_B;
            int64_t rate = float_divide(query_amt, diff_B);
            QOUT_XCH = rate;
            // return query result
            return accept(SBUF(query_out), __LINE__);
        }
        else
        if (BUFFER_EQUAL_20(cur, amm_cur_B) && BUFFER_EQUAL_20(isu, amm_cur_B + 20))
        {
            // currency B
            int64_t new_amt_B = float_sum(amm_amt_B, query_amt);
            int64_t calc_amt_A = float_divide(G, new_amt_B);
            int64_t diff_A = float_sum(amm_amt_A, float_negate(calc_amt_A));

            if (diff_A <= 0 || !float_compare(diff_A, 0, COMPARE_GREATER))
                NOPE("AMM: Error computing currency A.");

            COPY20(amm_cur_A, QOUT_CUR);
            COPY20(amm_cur_A + 20, QOUT_ISU);
            
            QOUT_AMT = diff_A;
            int64_t rate = float_divide(query_amt, diff_A);
            QOUT_XCH = rate;
            // return query result
            return accept(SBUF(query_out), __LINE__);
        }
        NOPE("AMM: Query failed. This AMM does not contain that currency.");
    }

    // execution to here means it's an incoming remit

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
    if (sent_currency_count > 2)
        NOPE("AMM: Send either 0, 1 or 2 currencies to use AMM.");
   
    // grab the user's liquidity tokens, if they have any
    state(SVAR(owner_lp), OTXNACC, 20);

    // First operation we'll deal with is a withdrawal. This happens if they send an empty remit.
    // All of their LP tokens are converted to currency and remitted back to them.
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

        int64_t ownership_percent = float_divide(owner_lp, total_lp);

        if (ownership_percent < 0)
            NOPE("AMM: Error computing ownership %");
        
        int64_t out_amt_A = float_multiply(amm_amt_A, ownership_percent);
        int64_t out_amt_B = float_multiply(amm_amt_B, ownership_percent);

        // clamp the amounts we're sending out
        int64_t send_all_A, send_all_B;
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

        total_lp -= owner_lp;
        if (total_lp <= 0)
        {
            // this is the final withdrawal, so remove setup information

            // delete all entries
            state_set(0,0, "TOT", 3);
            state_set(0,0, "A", 1);
            state_set(0,0, "B", 1);
            state_set(0,0, "CUR", 3);
            state_set(0,0, "G", 1);
        }
        else
        { 
            // compute the new values
            amm_amt_A = send_all_A ? amm_amt_A : float_sum(amm_amt_A, float_negate(out_amt_A));
            amm_amt_B = send_all_B ? amm_amt_B : float_sum(amm_amt_B, float_negate(out_amt_B));

            // compute new G
            int64_t new_G = float_multiply(amm_amt_A, amm_amt_B);
            if (new_G <= 0)
                NOPE("AMM: Internal error when calculating new G.");

            state_set(SVAR(new_G), "G", 1);
            
            // delete owner's entry
            state_set(0,0, OTXNACC, 20);
       
            // update balances
            state_set(SVAR(amm_amt_A), "A", 1);
            state_set(SVAR(amm_amt_B), "B", 1);
            
            // update total lp
            state_set(SVAR(total_lp), "TOT", 3);
        }

        // write amounts into remit
        float_sto(TXN_CUR_A, 49, ammcur +  0, 20, ammcur + 20, 20, out_amt_A, sfAmount);
        float_sto(TXN_CUR_B, 49, ammcur + 40, 20, ammcur + 60, 20, out_amt_B, sfAmount);

        DO_REMIT(0);
        DONE("AMM: Emitted withdraw.");
        return 0;
    }
    
    if (sent_currency_count != 1 && sent_currency_count != 2)
        NOPE("AMM: Send exactly 1 or 2 currencies.");

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

    // check if the AMM has been setup yet
    if (!already_setup)
    {
        // not setup, so whatever they sent is the first liquidity and sets the rate
        if (!has_sent_B)
            NOPE("AMM: Cannot setup new AMM without two currencies. Send two.");

        // the format for the packed data is
        // [ 20 byte currency code A ][ 20 byte issuer A or 0's for XAH ]
        // [ 20 byte currency code B ][ 20 byte issuer B or 0's for XAH ]
        
        // copy currency A into the array
        if (!sent_xah_A)
            COPY40(sent_cur_A + 8, ammcur);

        if (!sent_xah_B)
            COPY40(sent_cur_B + 8, ammcur + 40);

        if (sent_amt_A <= 0 || sent_amt_B <= 0)
            NOPE("AMM: Amounts must be positive non-zero.");

        // geometric mean constant
        int64_t G = float_multiply(sent_amt_A, sent_amt_B);

        TRACEXFL(G);
        TRACEXFL(sent_amt_A);
        TRACEXFL(sent_amt_B);
        state_set(SBUF(ammcur), "CUR", 3);
        state_set(SVAR(sent_amt_A), "A", 1);
        state_set(SVAR(sent_amt_B), "B", 1);
        state_set(SVAR(G), "G", 1);
    
        // set their liquidity tokens as a state entry
        owner_lp = 100;
        state_set(SVAR(owner_lp), OTXNACC, 20);
        state_set(SVAR(owner_lp), "TOT", 3);
        DONE("AMM: Created.");
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
        NOPE("AMM: Internal error.");

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

        int64_t new_amt_A = float_sum(amm_amt_A, sent_amt_A);
        int64_t new_amt_B = float_sum(amm_amt_B, sent_amt_B);

        int64_t amm_ratio = float_divide(amm_amt_A, amm_amt_B);
        int64_t sent_ratio = float_divide(sent_amt_A, sent_amt_B);

        int64_t diverge = float_divide(amm_ratio, sent_ratio);
        if (float_compare(diverge, 0, COMPARE_LESS))
            diverge = float_negate(diverge);

        int64_t new_G = float_multiply(new_amt_A, new_amt_B);

        if (float_compare(diverge, 6053837899185946624ULL /* 1% */, COMPARE_GREATER))
            NOPE("AMM: Divergence too great. Send amounts at the correct ratio.");
        

        // work out how many LP tokens they get
        // we'll work out their percentage ownership of A and B and average these
        
        int64_t ownership_A = float_divide(sent_amt_A, amm_amt_A);
        int64_t ownership_B = float_divide(sent_amt_B, amm_amt_B);

        if (ownership_A <= 0 || ownership_B <= 0)
            NOPE("AMM: Internal error.");

        int64_t ownership = float_divide(float_sum(ownership_A, ownership_B), 6090866696204910592ULL /* 2 */);

        int64_t new_lp = float_multiply(ownership, total_lp);

        if (total_lp + new_lp < total_lp ||
            owner_lp + new_lp < owner_lp)
            NOPE("AMM: Overflow error.");

        total_lp += new_lp;
        owner_lp += new_lp;

        amm_amt_A = float_sum(amm_amt_A, sent_amt_A);
        amm_amt_B = float_sum(amm_amt_B, sent_amt_B);

        if (amm_amt_A <= 0 || amm_amt_B <= 0)
            NOPE("AMM: Error crediting new balances to internal AMM state.");
        
        // ok credit their liquidity tokens
        if (state_set(SVAR(owner_lp), OTXNACC, 20) != 8 ||
            state_set(SVAR(total_lp), "TOT", 3) != 8 ||
            state_set(SVAR(amm_amt_A), "A", 1) != 8 ||
            state_set(SVAR(amm_amt_B), "B", 1) != 8 ||
            state_set(SVAR(new_G), "G", 1) != 8)
            NOPE("AMM: Error crediting liquidity (reserves?)");

        DONE("AMM: Liquidity added.");
    }

    // execution to here means they are not adding liquidity, but rather using the pool

    if (has_sent_A)
    {
        // sent only A, so return only B

        int64_t new_amt_A = float_sum(amm_amt_A, sent_amt_A);

        int64_t calc_amt_B = float_divide(G, new_amt_A);

        int64_t diff_B = float_sum(amm_amt_B, float_negate(calc_amt_B));

        if (diff_B <= 0 || !float_compare(diff_B, 0, COMPARE_GREATER))
            NOPE("AMM: Error computing currency B.");

        TRACEXFL(amm_amt_B);
        
        // subtract
        amm_amt_B = float_sum(amm_amt_B, float_negate(diff_B));

        if (amm_amt_B <= 0)
            NOPE("AMM: Error computing currency B.");

        TRACEXFL(amm_amt_A);
        TRACEXFL(amm_amt_B);

        state_set(SVAR(amm_amt_B), "B", 1);

        // write amount into remit (it's written to spot A in the out array, but it's currency B)
        float_sto(TXN_CUR_A, 49, ammcur +  40, 20, ammcur + 60, 20, diff_B, sfAmount);
    }
    else
    {
        int64_t new_amt_B = float_sum(amm_amt_B, sent_amt_B);

        int64_t calc_amt_A = float_divide(G, new_amt_B);

        int64_t diff_A = float_sum(amm_amt_A, float_negate(calc_amt_A));

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

        state_set(SVAR(amm_amt_A), "A", 1);

        // write amount into remit
        float_sto(TXN_CUR_A, 49, ammcur +  0, 20, ammcur + 20, 20, diff_A, sfAmount);
    }

    DO_REMIT(1);
    DONE("AMM: Emitted remit currency B.");
    return 0;
}

