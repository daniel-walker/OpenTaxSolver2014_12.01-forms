
struct xfdf_form_translation va760_2014[] = {
	{"HOH", IF_SET, "Head of Household", "Yes"},

	{"eic", IF_SET, "EIC", "savings"},
	{"eic_total", DOLLAR_ONLY, "EIC Amount", ""},

	{"exemptionsA", DOLLAR_ONLY, "Total Exempt", ""},
	{"MFJ", IF_SET, "Spouse if Filing Joint", "1"},
	{"x930", DOLLAR_ONLY, "X 930", ""},
	{"exemptionsB", DOLLAR_ONLY, "Total 1", ""},
	{"x800", DOLLAR_ONLY, "X 800", ""},

	{"status", DOLLAR_ONLY, "Filing Status", ""},
	{"1", DOLLAR_ONLY, "1. Federal Adjusted Gross Income", ""},
	{"2", DOLLAR_ONLY, "2. Addition from ADJ", ""},
	{"3", DOLLAR_ONLY, "Add Line 1 and 2", ""},
	{"4", DOLLAR_ONLY, "4. Total Age Deduction", ""},
	{"4y", DOLLAR_ONLY, "You Age Deduction", ""},
	{"4s", DOLLAR_ONLY, "Spouse Age Deduction", ""},
	{"5", DOLLAR_ONLY, "5. SSI", ""},
	{"6", DOLLAR_ONLY, "6. State Iome tax refund", ""},
	{"7", DOLLAR_ONLY, "7. Subtraction from ADJ", ""},
	{"8", DOLLAR_ONLY, "8. Add line 4,5,6 and 7", ""},
	{"9", DOLLAR_ONLY, "9. Virginia Adjusted Gross Income", ""},
	{"10", DOLLAR_ONLY, "10. Itemized Deductions", ""},
	{"11", DOLLAR_ONLY, "11. State and local income claimed", ""},
	{"12", DOLLAR_ONLY, "12. Subtract 11 from12 or enter Standard Deduction", ""},
	{"13", DOLLAR_ONLY, "Total Exemptions", ""},
	{"14", DOLLAR_ONLY, "14 Total Deductions from ADJ", ""},
	{"15", DOLLAR_ONLY, "15. Add Lines 12, 13 and 14", ""},
	{"16", DOLLAR_ONLY, "16. Virginia Taxable income", ""},
	{"17", DOLLAR_ONLY, "17. Amount of tax", ""},
	{"18a", DOLLAR_ONLY, "18. Spouse Tax Adjustment", ""},
	{"18", DOLLAR_ONLY, "18a STA Amount", ""},
	{"19", DOLLAR_ONLY, "19. Net Amount of Tax", ""},
	{"20", DOLLAR_ONLY, "Your Virginia Witholding", ""},
	{"20b", DOLLAR_ONLY, "Spouse Virginia Witholding", ""},
	{"21", DOLLAR_ONLY, "21. Estimated Payment Made", ""},
	{"22", DOLLAR_ONLY, "22. Overpayment credit from 2013", ""},
	{"23", DOLLAR_ONLY, "23. Extension Payment", ""},
	{"24", DOLLAR_ONLY, "24. Tax Credit LIC from ADJ", ""},
	{"25", DOLLAR_ONLY, "25. Credit for Taxes paid to another State", ""},
	{"26", DOLLAR_ONLY, "26. Credit for Political", ""},
	{"27", DOLLAR_ONLY, "27. Credit from SCH CR", ""},
	{"28", DOLLAR_ONLY, "28. Add Line 20a Through 27", ""},
	{"29", DOLLAR_ONLY, "29 Tax you owe", ""},
	{"30", DOLLAR_ONLY, "30. Overpayment", ""},
	{"31", DOLLAR_ONLY, "31. overpayment credited to next year", ""},
	{"32", DOLLAR_ONLY, "32. VCSP Contribution", ""},
	{"33", DOLLAR_ONLY, "33. Other Contributions from SCH VAC", ""},
	{"34", DOLLAR_ONLY, "34. Addition to TAx , Penalty and Interest", ""},
	{"35", DOLLAR_ONLY, "35 Consumer Use Tax", ""},
	{"36", DOLLAR_ONLY, "36. Add lines 31 Through 35", ""},
	{"37", DOLLAR_ONLY, "Amount Owed", ""},
	{"38", DOLLAR_ONLY, "38 Refund", ""},

	{NULL, DOLLAR_ONLY, NULL, NULL}
};
