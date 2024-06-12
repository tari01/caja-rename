/*
 * Copyright 2017-2023 Robert Tari <robert@tari.in>
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 3, as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranties of
 * MERCHANTABILITY, SATISFACTORY QUALITY, or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TITLECASE_H
#define TITLECASE_H

#include <glib.h>

#define SMALL "a|an|and|as|at|but|by|en|for|if|in|of|on|or|the|to|v\\.?|via|vs\\.?"
#define PUNCT "!\"#$%&'‘()*+,-./:;?@[\\]_`{|}~"

#if !GLIB_CHECK_VERSION (2, 74, 0)
    #define G_REGEX_DEFAULT 0
    #define G_REGEX_MATCH_DEFAULT 0
#endif

static GRegex *m_pSmallWords = NULL;
static GRegex *m_pInlinePeriod = NULL;
static GRegex *m_pUcElsewhere = NULL;
static GRegex *m_pCapFirst = NULL;
static GRegex *m_pSmallFirst = NULL;
static GRegex *m_pSmallLast = NULL;
static GRegex *m_pSubphrase = NULL;
static GRegex *m_pAPosSecond = NULL;
static GRegex *m_pAllCaps = NULL;
static GRegex *m_pUcInitials = NULL;
static GRegex *m_pMacMc = NULL;

static gboolean evalUp (const GMatchInfo *pInfo, GString *sResult, gpointer pData)
{
    gchar *sMatch = g_match_info_fetch (pInfo, 0);
    gchar *sUp = g_utf8_strup (sMatch, -1);
    g_free (sMatch);
    g_string_append (sResult, sUp);
    g_free (sUp);

    return FALSE;
}

static gboolean evalCap (const GMatchInfo *pInfo, GString *sResult, gpointer pData)
{
    gchar *sMatch = g_match_info_fetch (pInfo, 0);
    gchar *sFirst = g_utf8_offset_to_pointer (sMatch, 0);
    gchar *sRest = g_utf8_offset_to_pointer (sMatch, 1);
    gchar *sUp = g_utf8_strup (sFirst, -1);
    g_free (sMatch);
    gchar *sConcat = g_strconcat (sUp, sRest, NULL);
    g_free (sUp);
    g_string_append (sResult, sConcat);
    g_free (sConcat);

    return FALSE;
}

static gboolean evalConcat (const GMatchInfo *pInfo, GString *sResult, gpointer pData)
{
    gchar *sMatch1 = g_match_info_fetch (pInfo, 1);
    gchar *sMatch2 = g_match_info_fetch (pInfo, 2);
    gchar *sFirst = g_utf8_offset_to_pointer (sMatch2, 0);
    gchar *sRest = g_utf8_offset_to_pointer (sMatch2, 1);
    gchar *sUp = g_utf8_strup (sFirst, -1);
    gchar *sConcat = g_strconcat (sMatch1, sUp, sRest, NULL);
    g_free (sMatch2);
    g_free (sUp);
    g_free (sMatch1);
    g_string_append (sResult, sConcat);
    g_free (sConcat);

    return FALSE;
}

void titlecase_init ()
{
    GError *pError = NULL;
    m_pSmallWords = g_regex_new ("^("SMALL")$", G_REGEX_CASELESS, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pSmallWords)
    {
        g_error ("Panic while initialising m_pSmallWords: %s", pError->message);
        g_clear_error (&pError);
    }

    m_pInlinePeriod = g_regex_new ("[a-z][.][a-z]", G_REGEX_CASELESS, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pInlinePeriod)
    {
        g_error ("Panic while initialising m_pInlinePeriod: %s", pError->message);
        g_clear_error (&pError);
    }

    m_pUcElsewhere = g_regex_new ("["PUNCT"]*?[a-zA-Z]+[A-Z]+?", G_REGEX_DEFAULT, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pUcElsewhere)
    {
        g_error ("Panic while initialising m_pUcElsewhere: %s", pError->message);
        g_clear_error (&pError);
    }

    m_pCapFirst = g_regex_new ("^["PUNCT"]*?([A-Za-z])", G_REGEX_DEFAULT, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pCapFirst)
    {
        g_error ("Panic while initialising m_pCapFirst: %s", pError->message);
        g_clear_error (&pError);
    }

    m_pSmallFirst = g_regex_new ("^(["PUNCT"]*)("SMALL")\b", G_REGEX_CASELESS, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pSmallFirst)
    {
        g_error ("Panic while initialising m_pSmallFirst: %s", pError->message);
        g_clear_error (&pError);
    }

    m_pSmallLast = g_regex_new ("\b("SMALL")["PUNCT"]?$", G_REGEX_CASELESS, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pSmallLast)
    {
        g_error ("Panic while initialising m_pSmallLast: %s", pError->message);
        g_clear_error (&pError);
    }

    m_pSubphrase = g_regex_new ("([:.;?!\\-\\—][ ])("SMALL")", G_REGEX_DEFAULT, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pSubphrase)
    {
        g_error ("Panic while initialising m_pSubphrase: %s", pError->message);
        g_clear_error (&pError);
    }

    m_pAPosSecond = g_regex_new ("^[dol]{1}['‘]{1}[a-z]+(?:['s]{2})?$", G_REGEX_CASELESS, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pAPosSecond)
    {
        g_error ("Panic while initialising m_pAPosSecond: %s", pError->message);
        g_clear_error (&pError);
    }

    m_pAllCaps = g_regex_new ("^[A-Z\\s\\d"PUNCT"]+$", G_REGEX_DEFAULT, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pAllCaps)
    {
        g_error ("Panic while initialising m_pAllCaps: %s", pError->message);
        g_clear_error (&pError);
    }

    m_pUcInitials = g_regex_new ("^(?:[A-Z]{1}\\.{1}|[A-Z]{1}\\.{1}[A-Z]{1})+$", G_REGEX_DEFAULT, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pUcInitials)
    {
        g_error ("Panic while initialising m_pUcInitials: %s", pError->message);
        g_clear_error (&pError);
    }

    m_pMacMc = g_regex_new ("^([Mm]c|MC)(\\w.+)", G_REGEX_DEFAULT, G_REGEX_MATCH_DEFAULT, &pError);

    if (!m_pMacMc)
    {
        g_error ("Panic while initialising m_pMacMc: %s", pError->message);
        g_clear_error (&pError);
    }
}

gchar* titlecase_do (gchar *sText, gboolean bSmallFirstLast)
{
    gchar **lLines = g_regex_split_simple ("[\r\n]+", sText, G_REGEX_DEFAULT, G_REGEX_MATCH_DEFAULT);
    guint nLines = g_strv_length (lLines);
    GStrvBuilder *pProcessedBuilder = g_strv_builder_new ();

    for (guint nLine = 0; nLine < nLines; nLine++)
    {
        gboolean bAllCaps = g_regex_match (m_pAllCaps, lLines[nLine], G_REGEX_MATCH_DEFAULT, NULL);
        gchar **lWords = g_regex_split_simple ("[\t ]", lLines[nLine], G_REGEX_DEFAULT, G_REGEX_MATCH_DEFAULT);
        guint nWords = g_strv_length (lWords);
        GStrvBuilder *pLineBuilder = g_strv_builder_new ();

        for (guint nWord = 0; nWord < nWords; nWord++)
        {
            gchar *sWord = g_strdup (lWords[nWord]);

            if (bAllCaps)
            {
                gboolean bUcInitials = g_regex_match (m_pUcInitials, sWord, G_REGEX_MATCH_DEFAULT, NULL);

                if (bUcInitials)
                {
                    gchar *sNewWord = g_strdup (sWord);
                    g_strv_builder_add (pLineBuilder, sNewWord);

                    continue;
                }
            }

            gboolean bAPosSecond = g_regex_match (m_pAPosSecond, sWord, G_REGEX_MATCH_DEFAULT, NULL);

            if (bAPosSecond)
            {
                gchar *sChar0 = g_utf8_substring (sWord, 0, 1);
                gchar *sChar1 = g_utf8_substring (sWord, 1, 1);
                gchar *sChar2 = g_utf8_substring (sWord, 2, 1);
                gchar *sRest = g_utf8_substring (sWord, 3, -1);
                size_t nChar0Length = strlen (sChar0);
                char *pFound = strstr ("aeiouAEIOU", sChar0);
                gchar *sNew = NULL;

                if (nChar0Length == 1 && !pFound)
                {
                    sNew = g_utf8_strdown (sChar0, -1);
                }
                else
                {
                    sNew = g_utf8_strup (sChar0, -1);
                }

                g_free (sChar0);
                sChar0 = g_strdup (sNew);
                g_free (sNew);
                sNew = g_utf8_strup (sChar2, -1);
                g_free (sChar2);
                sChar2 = g_strdup (sNew);
                g_free (sNew);
                sNew = g_strconcat (sChar0, sChar1, sChar2, sRest, NULL);
                g_strv_builder_add (pLineBuilder, sNew);
                g_free (sChar0);
                g_free (sChar1);
                g_free (sChar2);
                g_free (sRest);

                continue;
            }

            GMatchInfo *pInfo = NULL;
            gboolean bMacMc = g_regex_match (m_pMacMc, sWord, G_REGEX_MATCH_DEFAULT, &pInfo);

            if (bMacMc)
            {
                gchar *sGroup1 = g_match_info_fetch (pInfo, 1);
                gchar *sGroup2 = g_match_info_fetch (pInfo, 2);
                g_match_info_free (pInfo);
                gchar *sCapitalised = g_utf8_strup (sGroup1, -1);
                g_free (sGroup1);
                gchar *sTitleCased = titlecase_do (sGroup2, bSmallFirstLast);
                g_free (sGroup2);
                gchar *sConcat = g_strconcat (sCapitalised, sTitleCased, NULL);
                g_free (sCapitalised);
                g_free (sTitleCased);
                g_strv_builder_add (pLineBuilder, sConcat);

                continue;
            }

            g_match_info_free (pInfo);
            gboolean bInlinePeriod = g_regex_match (m_pInlinePeriod, sWord, G_REGEX_MATCH_DEFAULT, NULL);
            gboolean bUcElsewhere = g_regex_match (m_pUcElsewhere, sWord, G_REGEX_MATCH_DEFAULT, NULL);

            if (bInlinePeriod || (!bAllCaps && bUcElsewhere))
            {
                gchar *sDup = g_strdup (sWord);
                g_strv_builder_add (pLineBuilder, sDup);

                continue;
            }

            gboolean bSmallWords = g_regex_match (m_pSmallWords, sWord, G_REGEX_MATCH_DEFAULT, NULL);

            if (bSmallWords)
            {
                gchar *sDown = g_utf8_strdown (sWord, -1);
                g_strv_builder_add (pLineBuilder, sDown);

                continue;
            }

            gchar *sSlash = g_strstr_len (sWord, -1, "/");
            gchar *sDoubleSlash = g_strstr_len (sWord, -1, "//");

            if (sSlash && !sDoubleSlash)
            {
                gchar **lSlashed = g_strsplit (sWord, "/", 0);
                guint nSlashedCount = g_strv_length (lSlashed);
                GStrvBuilder *pBuilder = g_strv_builder_new ();

                for (guint nSlashed = 0; nSlashed < nSlashedCount; nSlashed++)
                {
                    gchar *sSlashed = titlecase_do (lSlashed[nSlashed], FALSE);
                    g_strv_builder_add (pBuilder, sSlashed);
                }

                g_strfreev (lSlashed);
                GStrv lSlashedNew = g_strv_builder_end (pBuilder);
                g_strv_builder_unref (pBuilder);
                gchar *sSlashedNew = g_strjoinv ("/", lSlashedNew);
                g_strv_builder_add (pLineBuilder, sSlashedNew);
                g_strfreev (lSlashedNew);

                continue;
            }

            gchar *sHyphen = g_strstr_len (sWord, -1, "-");

            if (sHyphen)
            {
                gchar **lHyphenated = g_strsplit (sWord, "-", 0);
                guint nHyphenatedCount = g_strv_length (lHyphenated);
                GStrvBuilder *pBuilder = g_strv_builder_new ();

                for (guint nHyphenated = 0; nHyphenated < nHyphenatedCount; nHyphenated++)
                {
                    gchar *sHyphenated = titlecase_do (lHyphenated[nHyphenated], bSmallFirstLast);
                    g_strv_builder_add (pBuilder, sHyphenated);
                }

                g_strfreev (lHyphenated);
                GStrv lHyphenatedNew = g_strv_builder_end (pBuilder);
                g_strv_builder_unref (pBuilder);
                gchar *sHyphenatedNew = g_strjoinv ("-", lHyphenatedNew);
                g_strv_builder_add (pLineBuilder, sHyphenatedNew);
                g_strfreev (lHyphenatedNew);

                continue;
            }

            if (bAllCaps)
            {
                gchar *sNewWord = g_utf8_strdown (sWord, -1);
                g_free (sWord);
                sWord = g_strdup (sNewWord);
                g_free (sNewWord);
            }

            gboolean bCapFirst = g_regex_match (m_pCapFirst, sWord, G_REGEX_MATCH_DEFAULT, NULL);
            gchar *sNewWord = NULL;

            if (bCapFirst)
            {
                GError *pError = NULL;
                sNewWord = g_regex_replace_eval (m_pCapFirst, sWord, -1, 0, G_REGEX_MATCH_DEFAULT, evalUp, NULL, &pError);

                if (pError)
                {
                    g_error ("Panic calling evalCase: %s", pError->message);
                    g_error_free (pError);
                }
            }
            else
            {
                sNewWord = g_strdup (sWord);
            }

            g_strv_builder_add (pLineBuilder, sNewWord);
            g_free (sWord);
        }

        g_strfreev (lWords);
        GStrv lWordsNew = g_strv_builder_end (pLineBuilder);
        g_strv_builder_unref (pLineBuilder);
        gchar *sResult = g_strjoinv (" ", lWordsNew);
        g_strfreev (lWordsNew);

        if (bSmallFirstLast)
        {
            gboolean bSmallFirst = g_regex_match (m_pSmallFirst, sResult, G_REGEX_MATCH_DEFAULT, NULL);

            if (bSmallFirst)
            {
                GError *pError = NULL;
                gchar *sNewResult = g_regex_replace_eval (m_pSmallFirst, sResult, -1, 0, G_REGEX_MATCH_DEFAULT, evalConcat, NULL, &pError);

                if (pError)
                {
                    g_error ("Panic in m_pSmallFirst eval: %s", pError->message);
                    g_error_free (pError);
                }

                g_free (sResult);
                sResult = g_strdup (sNewResult);
                g_free (sNewResult);
            }

            gboolean bSmallLast = g_regex_match (m_pSmallLast, sResult, G_REGEX_MATCH_DEFAULT, NULL);

            if (bSmallLast)
            {
                GError *pError = NULL;
                gchar *sNewResult = g_regex_replace_eval (m_pSmallLast, sResult, -1, 0, G_REGEX_MATCH_DEFAULT, evalCap, NULL, &pError);

                if (pError)
                {
                    g_error ("Panic in m_pSmallLast eval: %s", pError->message);
                    g_error_free (pError);
                }

                g_free (sResult);
                sResult = g_strdup (sNewResult);
                g_free (sNewResult);
            }
        }

        gboolean bSubphrase = g_regex_match (m_pSubphrase, sResult, G_REGEX_MATCH_DEFAULT, NULL);

        if (bSubphrase)
        {
            GError *pError = NULL;
            gchar *sNewResult = g_regex_replace_eval (m_pSubphrase, sResult, -1, 0, G_REGEX_MATCH_DEFAULT, evalConcat, NULL, &pError);

            if (pError)
            {
                g_error ("Panic in m_pSubphrase eval: %s", pError->message);
                g_error_free (pError);
            }

            g_free (sResult);
            sResult = g_strdup (sNewResult);
            g_free (sNewResult);
        }

        g_strv_builder_add (pProcessedBuilder, sResult);
    }

    g_strfreev (lLines);
    GStrv lProcessed = g_strv_builder_end (pProcessedBuilder);
    g_strv_builder_unref (pProcessedBuilder);
    gchar *sProcessed = g_strjoinv ("\n", lProcessed);
    g_strfreev (lProcessed);

    return sProcessed;
}

void titlecase_finish ()
{
    g_regex_unref (m_pSmallWords);
    g_regex_unref (m_pInlinePeriod);
    g_regex_unref (m_pUcElsewhere);
    g_regex_unref (m_pCapFirst);
    g_regex_unref (m_pSmallFirst);
    g_regex_unref (m_pSmallLast);
    g_regex_unref (m_pSubphrase);
    g_regex_unref (m_pAPosSecond);
    g_regex_unref (m_pAllCaps);
    g_regex_unref (m_pUcInitials);
    g_regex_unref (m_pMacMc);
}

#endif
