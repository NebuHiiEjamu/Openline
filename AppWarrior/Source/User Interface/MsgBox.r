/*
 * MsgBox.r
 * (c)1998 Hotline Communications
 */

type 'MSGB'
{			
	integer																// icon ID
		NoIcon=0,StopIcon=1000,NoteIcon=1001,CautionIcon=1002,QuestionIcon=1003;
	integer																// picture ID
		NoPicture=0;
	integer																// sound ID
		NoSound=0,Beep=1;
	integer																// text style ID
		DefaultTextStyle=0;
	integer																// button text style ID
		DefaultTextStyle=0;
	integer																// script
		RomanScript=0;
	pstring																// title
		NoTitle="";
	pstring																// message
		NoMessage="";
	pstring																// button 1
		OK="OK";
	pstring																// button 2
		NoButton="",Cancel="Cancel";
	pstring																// button 3
		NoButton="";
};
