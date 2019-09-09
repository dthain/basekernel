/*
Copyright (C) 2016-2019 The University of Notre Dame
This software is distributed under the GNU General Public License.
See the file LICENSE for details.
*/

/*
A trivial user level program to try out basic system calls.
*/

#include "library/syscalls.h"
#include "library/string.h"

char *shakespeare[] = {
	"[Horatio] Now cracke a Noble heart:",
	"Goodnight sweet Prince,",
	"And flights of Angels sing thee to thy rest,",
	"Why do's the Drumme come hither?",
	"Enter Fortinbras and English Ambassador, with Drumme,",
	"Colours, and Attendants.",
	"[Fortinbras] Where is this sight?",
	"[Horatio] What is it ye would see;",
	"If ought of woe, or wonder, cease your search.",
	"[Fortinbras] His quarry cries on hauocke. Oh proud death,",
	"What feast is toward in thine eternall Cell.",
	"That thou so many Princes, at a shoote,",
	"So bloodily hast strooke.",
	"[Ambassador] The sight is dismall,",
	"And our affaires from England come too late,",
	"The eares are senselesse that should giue vs hearing,",
	"To tell him his command'ment is fulfill'd,",
	"That Rosincrance and Guildensterne are dead:",
	"Where should we haue our thankes?",
	"[Horatio] Not from his mouth,",
	"Had it th'abilitie of life to thanke you:",
	"He neuer gaue command'ment for their death.",
	"But since so iumpe vpon this bloodie question,",
	"You from the Polake warres, and you from England",
	"Are heere arriued. Giue order that these bodies",
	"High on a stage be placed to the view,",
	"And let me speake to th'yet vnknowing world,",
	"How these things came about. So shall you heare",
	"Of carnall, bloudie, and vnnaturall acts,",
	"Of accidentall iudgements, casuall slaughters",
	"Of death's put on by cunning, and forc'd cause,",
	"And in this vpshot, purposes mistooke,",
	"Falne on the Inuentors heads. All this can I",
	"Truly deliuer.",
	"[Fortinbras] Let vs hast to heare it,",
	"And call the Noblest to the Audience.",
	"For me, with sorrow, I embrace my Fortune,",
	"I haue some Rites of memory in this Kingdome,",
	"Which are ro claime, my vantage doth",
	"Inuite me,",
	"[Horatio] Of that I shall haue alwayes cause to speake,",
	"And from his mouth",
	"Whose voyce will draw on more:",
	"But let this same be presently perform'd,",
	"Euen whiles mens mindes are wilde,",
	"Lest more mischance",
	"On plots, and errors happen.",
	"[Fortinbras] Let foure Captaines",
	"Beare Hamlet like a Soldier to the Stage,",
	"For he was likely, had he beene put on",
	"To haue prou'd most royally:",
	"And for his passage,",
	"The Souldiours Musicke, and the rites of Warre",
	"Speake lowdly for him.",
	"Take vp the body; Such a sight as this",
	"Becomes the Field, but heere shewes much amis.",
	"Go, bid the Souldiers shoote.",
	"Exeunt Marching: after the which, a Peale of",
	"Ordenance are shot off.",
};

int main(int argc, char *argv[])
{
	int i;
	for(i = 0; i < sizeof(shakespeare) / sizeof(char *); i++) {
		printf("%s\n", shakespeare[i]);
		syscall_process_sleep(1000);
	}

	return 0;
}
