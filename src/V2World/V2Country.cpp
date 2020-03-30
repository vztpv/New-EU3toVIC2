/*Copyright (c) 2014 The Paradox Game Converters Project

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.*/


#include "V2Country.h"
#include <algorithm>
#include <math.h>
#include <float.h>
#include <io.h>
#include <fstream>
#include <sstream>
#include <queue>
#include "../Log.h"
#include "../Configuration.h"
#include "../Parsers/Parser.h"
#include "../EU3World/EU3World.h"
#include "../EU3World/EU3Province.h"
#include "../EU3World/EU3Relations.h"
#include "../EU3World/EU3Loan.h"
#include "../EU3World/EU3Leader.h"
#include "V2World.h"
#include "V2State.h"
#include "V2Province.h"
#include "V2Relations.h"
#include "V2Army.h"
#include "V2Reforms.h"
#include "V2Creditor.h"
#include "V2Leader.h"
#include "V2Pop.h"



const int MONEYFACTOR = 30;	// ducat to pound conversion rate


V2Country::V2Country(string _tag, string _commonCountryFile, vector<V2Party*> _parties, V2World* _theWorld, bool _newCountry, bool _dynamicCountry)
{
	theWorld			= _theWorld;
	newCountry		= _newCountry;
	dynamicCountry	= _dynamicCountry;

	tag					= _tag;
	commonCountryFile	= localisation.convertCountryFileName(_commonCountryFile);
	int pos = commonCountryFile.find_first_of(":");
	while (pos != string::npos)
	{
		commonCountryFile.replace(pos, 1, ";");
		pos = commonCountryFile.find_first_of(":");
	}
	parties				= _parties;
	rulingParty			= "";

	states.clear();
	provinces.clear();

	for (unsigned int i = 0; i < VANILLA_naval_exercises; i++)
	{
		vanillaInventions[i] = illegal;
	}
	for (unsigned int i = 0; i < HOD_naval_exercises; i++)
	{
		HODInventions[i] = illegal;
	}
	for (unsigned int i = 0; i < HOD_NNM_naval_exercises; i++)
	{
		HODNNMInventions[i] = illegal;
	}

	leadership		= 0.0;
	plurality		= 0.0;
	capital			= 0;
	diploPoints		= 0.0;
	badboy			= 0.0;
	prestige			= 0.0;
	money				= 0.0;
	techSchool		= "traditional_academic";
	researchPoints	= 0.0;
	civilized		= false;
	primaryCulture	= "";
	religion			= "";
	government		= "";
	nationalValue	= "";
	lastBankrupt	= date();
	bankReserves	= 0.0;
	literacy			= 0.0;

	acceptedCultures.clear();
	techs.clear();
	reactionaryIssues.clear();
	conservativeIssues.clear();
	liberalIssues.clear();
	relations.clear();
	armies.clear();
	creditors.clear();
	leaders.clear();

	reforms		= NULL;
	srcCountry	= NULL;

	upperHouseReactionary	= 10;
	upperHouseConservative	= 65;
	upperHouseLiberal			= 25;

	uncivReforms = NULL;

	if (parties.empty())
	{	// No parties are specified. Generate some default parties for this country.
		const std::vector<std::string> ideologies = 
				{ "conservative", "liberal", "reactionary", "socialist", "communist", "anarcho_liberal", "fascist" };
		const std::vector<std::string> partyNames =
				{ "Conservative Party", "Liberal Party", "National Party", 
				  "Socialist Party", "Communist Party", "Radical Party", "Fascist Party" };
		for (size_t i = 0; i < ideologies.size(); ++i)
		{
			std::string partyKey = tag + '_' + ideologies[i];
			parties.push_back(new V2Party(partyKey, ideologies[i]));
			localisation.SetPartyKey(i, partyKey);
			localisation.SetPartyName(i, "english", partyNames[i]);
		}
	}

	for (int i = 0; i < num_reg_categories; ++i)
	{
		unitNameCount[i] = 0;
	}

	numFactories = 0;
}


void V2Country::output() const
{
	if(!dynamicCountry)
 	{
		FILE* output;
		if (fopen_s(&output, ("Output\\" + Configuration::getOutputName() + "\\history\\countries\\" + filename).c_str(), "w") != 0)
		{
			LOG(LogLevel::Error) << "Could not create country history file " << filename;
			exit(-1);
		}

		if (capital > 0)
		{
			fprintf(output, "capital=%d\n", capital);
		}
		if (primaryCulture.size() > 0)
		{
			fprintf(output, "primary_culture = %s\n", primaryCulture.c_str());
		}
		for (set<string>::iterator i = acceptedCultures.begin(); i != acceptedCultures.end(); i++)
		{
			fprintf(output, "culture = %s\n", i->c_str());
		}
		if (religion != "")
		{
			fprintf(output, "religion = %s\n", religion.c_str());
		}
		if (government != "")
		{
			fprintf(output, "government = %s\n", government.c_str());
		}
		if (plurality > 0.0)
		{
			fprintf(output, "plurality=%f\n", plurality);
		}
		fprintf(output, "nationalvalue=%s\n", nationalValue.c_str());
		fprintf(output, "literacy=%f\n", literacy);
		if (civilized)
		{
			fprintf(output, "civilized=yes\n");
		}
		fprintf(output, "\n");
		fprintf(output, "ruling_party=%s\n", rulingParty.c_str());
		fprintf(output, "upper_house=\n");
		fprintf(output, "{\n");
		fprintf(output, "	fascist = 0\n");
		fprintf(output, "	liberal = %d\n", upperHouseLiberal);
		fprintf(output, "	conservative = %d\n", upperHouseConservative);
		fprintf(output, "	reactionary = %d\n", upperHouseReactionary);
		fprintf(output, "	anarcho_liberal = 0\n");
		fprintf(output, "	socialist = 0\n");
		fprintf(output, "	communist = 0\n");
		fprintf(output, "}\n");
		fprintf(output, "\n");
		fprintf(output, "# Starting Consciousness\n");
		fprintf(output, "consciousness = 0\n");
		fprintf(output, "nonstate_consciousness = 0\n");
		fprintf(output, "\n");
		outputTech(output);
		if (reforms != NULL)
		{
			reforms->output(output);
		}
		if (!civilized)
		{
			if (uncivReforms != NULL)
			{
				uncivReforms->output(output);
			}
		}
		fprintf(output, "prestige=%f\n", prestige);

		fprintf(output, "\n");
		fprintf(output, "# Social Reforms\n");
		fprintf(output, "wage_reform = no_minimum_wage\n");
		fprintf(output, "work_hours = no_work_hour_limit\n");
		fprintf(output, "safety_regulations = no_safety\n");
		fprintf(output, "health_care = no_health_care\n");
		fprintf(output, "unemployment_subsidies = no_subsidies\n");
		fprintf(output, "pensions = no_pensions\n");
		fprintf(output, "school_reforms = no_schools\n");

		if (reforms != NULL)
		{
			reforms->output(output);
		}

		/*fprintf(output, "	schools=\"%s\"\n", techSchool.c_str());*/

		fprintf(output, "oob = \"%s\"\n", (tag + "_OOB.txt").c_str());

		fclose(output);

		outputOOB();
	}

	if (newCountry)
	{
		// Output common country file. 
		std::ofstream commonCountryOutput("Output\\" + Configuration::getOutputName() + "\\common\\countries\\" + commonCountryFile);
		if (!commonCountryOutput.is_open())
		{
			LOG(LogLevel::Error) << "Could not open Output\\" + Configuration::getOutputName() + "\\common\\countries\\" + commonCountryFile;
			exit(-1);
		}
		commonCountryOutput << "graphical_culture = UsGC\n";	// default to US graphics
		commonCountryOutput << "color = { " << color << " }\n";
		for (auto party : parties)
		{
			commonCountryOutput	<< '\n'
										<< "party = {\n"
										<< "    name = \"" << party->name << "\"\n"
										<< "    start_date = " << party->start_date << '\n'
										<< "    end_date = " << party->end_date << "\n\n"
										<< "    ideology = " << party->ideology << "\n\n"
										<< "    economic_policy = " << party->economic_policy << '\n'
										<< "    trade_policy = " << party->trade_policy << '\n'
										<< "    religious_policy = " << party->religious_policy << '\n'
										<< "    citizenship_policy = " << party->citizenship_policy << '\n'
										<< "    war_policy = " << party->war_policy << '\n'
										<< "}\n";
		}
	}
}


void V2Country::outputToCommonCountriesFile(FILE* output) const
{
	fprintf(output, "%s = \"countries%s\"\n", tag.c_str(), commonCountryFile.c_str());
}


void V2Country::outputLocalisation(FILE* output) const
{
	std::ostringstream localisationStream;
	localisation.WriteToStream(localisationStream);
	std::string localisationString = localisationStream.str();
	fwrite(localisationString.c_str(), sizeof(std::string::value_type), localisationString.size(), output);
}


void V2Country::outputTech(FILE* output) const
{
	fprintf(output, "\n");
	fprintf(output, "# Technologies\n");
	for (vector<string>::const_iterator itr = techs.begin(); itr != techs.end(); ++itr)
	{
		fprintf(output, itr->c_str()); fprintf(output, " = 1\n");
	}
}


void V2Country::outputElection(FILE* output) const
{
	date electionDate = date("1836.1.1");

	if (electionDate.month == 12)
	{
		electionDate.month = 1;
		electionDate.year++;
	}
	else
	{
		electionDate.month++;
	}
	electionDate.year -= 4;
	fprintf(output, "	last_election=%s\n", electionDate.toString().c_str());
}


void V2Country::outputOOB() const
{
	FILE* output;
	if (fopen_s(&output, ("Output\\" + Configuration::getOutputName() + "\\history\\units\\" + tag + "_OOB.txt").c_str(), "w") != 0)
	{
		LOG(LogLevel::Error) << "Could not create OOB file " << (tag + "_OOB.txt");
		exit(-1);
	}

	fprintf(output, "#Sphere of Influence\n");
	fprintf(output, "\n");
	for (map<string, V2Relations*>::const_iterator relationsItr = relations.begin(); relationsItr != relations.end(); relationsItr++)
	{
		relationsItr->second->output(output);
	}

	fprintf(output, "\n");
	fprintf(output, "#Leaders\n");
	for (vector<V2Leader*>::const_iterator itr = leaders.begin(); itr != leaders.end(); ++itr)
	{
		(*itr)->output(output);
	}

	fprintf(output, "\n");
	fprintf(output, "#Armies\n");
	for (vector<V2Army*>::const_iterator itr = armies.begin(); itr != armies.end(); ++itr)
	{
		(*itr)->output(output);
	}

	fclose(output);
}


void V2Country::initFromEU3Country(const EU3Country* _srcCountry, vector<string> outputOrder, const CountryMapping& countryMap, cultureMapping cultureMap, religionMapping religionMap, unionCulturesMap unionCultures, governmentMapping governmentMap, inverseProvinceMapping inverseProvinceMap, vector<V2TechSchool> techSchools, map<int, int>& leaderMap, const V2LeaderTraits& lt, const EU3RegionsMapping& regionsMap)
{
	srcCountry = _srcCountry;

	struct _finddata_t	fileData;
	intptr_t					fileListing;
	string filesearch = ".\\blankMod\\output\\history\\countries\\" + tag + "*.txt";
	if ((fileListing = _findfirst(filesearch.c_str(), &fileData)) != -1L)
	{
		filename = fileData.name;
	}
	_findclose(fileListing);
	if (filename == "")
	{
		string filesearch = Configuration::getV2Path() + "\\history\\countries\\" + tag + "*.txt";
		if ((fileListing = _findfirst(filesearch.c_str(), &fileData)) != -1L)
		{
			filename = fileData.name;
			}
		_findclose(fileListing);
	}
	if (filename == "")
	{
		string countryName = commonCountryFile;
		int lastSlash = countryName.find_last_of("/");
		countryName = countryName.substr(lastSlash + 1, countryName.size());
		filename = tag + " - " + countryName;
	}

	// Color
	color = srcCountry->getColor();

	// Localisation
	localisation.SetTag(tag);
	localisation.ReadFromCountry(*srcCountry);

	// Capital
	int oldCapital = srcCountry->getCapital();
	inverseProvinceMapping::iterator itr = inverseProvinceMap.find(oldCapital);
	if (itr != inverseProvinceMap.end())
	{
		capital = itr->second[0];
	}

	// tech group
	if (	(srcCountry->getTechGroup() == "western") || (srcCountry->getTechGroup() == "latin") ||
			(srcCountry->getTechGroup() == "eastern") || (srcCountry->getTechGroup() == "ottoman"))
	{
		civilized = true;
	}
	else
	{
		civilized = false;
	}

	// religion
	string srcReligion = srcCountry->getReligion();
	if (srcReligion.size() > 0)
	{
		religionMapping::iterator i = religionMap.find(srcReligion);
		if (i != religionMap.end())
		{
			religion = i->second;
		}
		else
		{
			LOG(LogLevel::Warning) << "No religion mapping defined for " << srcReligion << " (" << _srcCountry->getTag() << " -> " << tag << ')';
		}
	}

	// primary culture
	string srcCulture = srcCountry->getPrimaryCulture();
	if (srcCulture.size() > 0)
	{
		bool matched = false;
		for (cultureMapping::iterator i = cultureMap.begin(); (i != cultureMap.end()) && (!matched); i++)
		{
			if (i->srcCulture == srcCulture)
			{
				bool match = true;
				for (vector<distinguisher>::iterator j = i->distinguishers.begin(); j != i->distinguishers.end(); j++)
				{
					if (j->first == DTOwner)
					{
						if (tag != j->second)
						{
								match = false;
						}
					}
					else if (j->first == DTReligion)
					{
						if (religion != j->second)
						{
							match = false;
						}
					}
					else if (j->first == DTRegion)
					{
						auto regions = regionsMap.find(oldCapital);
						if ((regions == regionsMap.end()) || (regions->second.find(j->second) == regions->second.end()))
						{
							match = false;
						}
						else
						{
							match = true;
						}
					}
					else
					{
						LOG(LogLevel::Warning) << "Unhandled distinguisher type in culture rules";
					}
				}
				if (match)
				{
					primaryCulture = i->dstCulture;
					matched = true;
				}
			}
		}
		if (!matched)
		{
			LOG(LogLevel::Warning) << "No culture mapping defined for " << srcCulture << " (" << srcCountry->getTag() << " -> " << tag << ')';
		}
	}

	//accepted cultures
	vector<string> srcAceptedCultures = srcCountry->getAcceptedCultures();
	unionCulturesMap::iterator unionItr = unionCultures.find(srcCountry->getTag());
	if (unionItr != unionCultures.end())
	{
		for (vector<string>::iterator j = unionItr->second.begin(); j != unionItr->second.end(); j++)
		{
			srcAceptedCultures.push_back(*j);
		}
	}
	for (vector<string>::iterator i = srcAceptedCultures.begin(); i != srcAceptedCultures.end(); i++)
	{
		bool matched = false;
		for (cultureMapping::iterator j = cultureMap.begin(); (j != cultureMap.end()) && (!matched); j++)
		{
			if (j->srcCulture == *i)
			{
				bool match = true;
				for (vector<distinguisher>::iterator k = j->distinguishers.begin(); k != j->distinguishers.end(); k++)
				{
					if (k->first == DTOwner)
					{
						if (tag != k->second)
						{
							match = false;
						}
					}
					else if (k->first == DTReligion)
					{
						if (religion != k->second)
						{
							match = false;
						}
					}
					else if (k->first == DTRegion)
					{
						auto regions = regionsMap.find(oldCapital);
						if ((regions == regionsMap.end()) || (regions->second.find(k->second) == regions->second.end()))
						{
							match = false;
						}
						else
						{
							match = true;
						}
					}
					else
					{
						LOG(LogLevel::Warning) << "Unhandled distinguisher type in culture rules";
					}
				}
				if (match)
				{
					acceptedCultures.insert(j->dstCulture);
					matched = true;
				}
			}
		}
		if (!matched)
		{
			LOG(LogLevel::Warning) << "No culture mapping defined for " << *i << " (" << srcCountry->getTag() << " -> " << tag << ')';
		}
	}

	// Prestige
	prestige		+= srcCountry->getPrestige() + 100;
	prestige		+= srcCountry->getCulture();
	prestige		+= srcCountry->getArmyTradition() + srcCountry->getNavyTradition();

	// Government
	string srcGovernment = srcCountry->getGovernment();
	if (srcGovernment.size() > 0)
	{
		governmentMapping::iterator i = governmentMap.find(srcGovernment);
		if (i != governmentMap.end())
		{
			government = i->second;
		}
		else
		{
			LOG(LogLevel::Warning) << "No government mapping defined for " << srcGovernment << " (" << srcCountry->getTag() << " -> " << tag << ')';
		}
	}

	//  Politics
	double liberalEffect			= 0.0;
	double reactionaryEffect	= 0.0;
	if (srcCountry->getCentralizationDecentralization() > 0)
	{
		liberalEffect += srcCountry->getCentralizationDecentralization();
	}
	if (srcCountry->getAristocracyPlutocracy() < 0)
	{
		reactionaryEffect -= srcCountry->getAristocracyPlutocracy();
	}
	else
	{
		liberalEffect += srcCountry->getAristocracyPlutocracy();
	}
	if (srcCountry->getSerfdomFreesubjects() < 0)
	{
		reactionaryEffect -= srcCountry->getSerfdomFreesubjects();
	}
	else
	{
		liberalEffect += srcCountry->getSerfdomFreesubjects();
	}
	if (srcCountry->getInnovativeNarrowminded() < 0)
	{
		liberalEffect -= srcCountry->getInnovativeNarrowminded();
	}
	if (srcCountry->getMercantilismFreetrade() > 0)
	{
		liberalEffect += srcCountry->getMercantilismFreetrade();
	}
	upperHouseReactionary	= static_cast<int>(5  + reactionaryEffect);
	upperHouseLiberal			= static_cast<int>(10 + liberalEffect);
	upperHouseConservative	= static_cast<int>(85 - (reactionaryEffect + liberalEffect));
	LOG(LogLevel::Debug) << tag << " has an Upper House of " << upperHouseReactionary << " reactionary, "
																				<< upperHouseConservative << " conservative, and "
																				<< upperHouseLiberal << " liberal";
	
	string idealogy;
	if (liberalEffect >= 2 * reactionaryEffect)
	{
		idealogy = "liberal";
	}
	else if (reactionaryEffect >= 2 * liberalEffect)
	{
		idealogy = "reactionary";
	}
	else
	{
		idealogy = "conservative";
	}
	for (vector<V2Party*>::iterator i = parties.begin(); i != parties.end(); i++)
	{
		if ((*i)->isActiveOn(date("1836.1.1")) && ((*i)->ideology == idealogy))
		{
			rulingParty = (*i)->name;
			break;
		}
	}
	if (rulingParty == "")
	{
		for (vector<V2Party*>::iterator i = parties.begin(); i != parties.end(); i++)
		{
			if ((*i)->isActiveOn(date("1836.1.1")))
			{
				rulingParty = (*i)->name;
				break;
			}
		}
	}
	LOG(LogLevel::Debug) << tag << " ruling party is " << rulingParty;

	// Reforms
	reforms		=  new V2Reforms(this, srcCountry);

	// Relations
	vector<EU3Relations*> srcRelations = srcCountry->getRelations();
	if (srcRelations.size() > 0)
	{
		for (vector<EU3Relations*>::iterator itr = srcRelations.begin(); itr != srcRelations.end(); ++itr)
		{
			const std::string& V2Tag = countryMap[(*itr)->getCountry()];
			if (!V2Tag.empty())
			{
				V2Relations* v2r = new V2Relations(V2Tag, *itr);
				relations.insert(make_pair(V2Tag, v2r));
			}
		}
	}

	bankReserves = MONEYFACTOR *srcCountry->inflationAdjust(srcCountry->getEstimatedMonthlyIncome());
	if (srcCountry->hasNationalIdea("national_bank"))
	{
		bankReserves *= 6.0;
	}

	// Literacy
	double innovationFactor	= 5 * (5 - srcCountry->getInnovativeNarrowminded());
	double serfdomFactor		= 5 * (5 + srcCountry->getSerfdomFreesubjects());
	literacy = (innovationFactor + serfdomFactor) * 0.004;
	if ( (srcCountry->getReligion() == "Protestant") || (srcCountry->getReligion() == "Confucianism") || (srcCountry->getReligion() == "Reformed") )
	{
		literacy += 0.05;
	}
	if ( srcCountry->hasNationalIdea("bureaucracy") )
	{
		literacy += 0.04;
	}
	if ( srcCountry->hasNationalIdea("liberty_egalite_fraternity") )
	{
		literacy += 0.04;
	}
	if ( srcCountry->hasNationalIdea("church_attendance_duty") )
	{
		literacy += 0.04;
	}
	if ( srcCountry->hasNationalIdea("scientific_revolution") )
	{
		literacy += 0.04;
	}
	if ( srcCountry->hasModifier("the_school_establishment_act") )
	{
		literacy += 0.04;
	}
	if ( srcCountry->hasModifier("sunday_schools") )
	{
		literacy += 0.04;
	}
	if ( srcCountry->hasModifier("the_education_act") )
	{
		literacy += 0.04;
	}
	if ( srcCountry->hasModifier("monastic_education_system") )
	{
		literacy += 0.04;
	}
	if ( srcCountry->hasModifier("western_embassy_mission") )
	{
		literacy += 0.04;
	}
	int numProvinces = 0;
	int numUniversities = 0;
	vector<EU3Province*> provinces = srcCountry->getProvinces();
	numUniversities = provinces.size();
	for (vector<EU3Province*>::iterator i = provinces.begin(); i != provinces.end(); i++)
	{
		if ( (*i)->hasBuilding("university") )
		{
			numUniversities++;
		}
	}
	double universityBonus1;
	if (numProvinces > 0)
	{
		universityBonus1 = numUniversities / numProvinces;
	}
	else
	{
		universityBonus1 = 0;
	}
	double universityBonus2	= numUniversities * 0.01;
	double universityBonus	= max(universityBonus1, universityBonus2);
	if (universityBonus > 0.2)
	{
		universityBonus = 0.2;
	}
	literacy += universityBonus;
	string techGroup = srcCountry->getTechGroup();
	if ( (techGroup == "western") || (techGroup == "latin") || (techGroup == "eastern") || (techGroup == "ottoman") )
	{
		literacy += 0.1;
	}
	if (literacy > Configuration::getMaxLiteracy())
	{
		literacy = Configuration::getMaxLiteracy();
	}
	LOG(LogLevel::Debug) << "Setting literacy for " << tag << " to " << literacy;

	// Tech School
	double armyInvestment			= srcCountry->getArmyInvestment();
	double navyInvestment			= srcCountry->getNavyInvestment();
	double commerceInvestment		= srcCountry->getCommerceInvestment();
	double industryInvestment		= srcCountry->getIndustryInvestment();
	double cultureInvestment		= srcCountry->getCultureInvestment();

	vector<EU3Province*> srcProvinces = srcCountry->getProvinces();
	for(unsigned int j = 0; j < srcProvinces.size(); j++)
	{
		if (srcProvinces[j]->hasBuilding("weapons"))
		{
			armyInvestment += 50;
		}
		if (srcProvinces[j]->hasBuilding("wharf"))
		{
			navyInvestment += 50;
		}
		if (srcProvinces[j]->hasBuilding("refinery"))
		{
			commerceInvestment += 50;
		}
		if (srcProvinces[j]->hasBuilding("textile"))
		{
			industryInvestment += 50;
		}
		if (srcProvinces[j]->hasBuilding("university"))
		{
			cultureInvestment += 50;
		}
	}

	double totalInvestment	 = armyInvestment + navyInvestment + commerceInvestment + industryInvestment + cultureInvestment;
	armyInvestment				/= totalInvestment;
	navyInvestment				/= totalInvestment;
	commerceInvestment		/= totalInvestment;
	industryInvestment		/= totalInvestment;
	cultureInvestment			/= totalInvestment;

	double lowestScore = 1.0;
	string bestSchool = "traditional_academic";

	for (unsigned int j = 0; j < techSchools.size(); j++)
	{
		double newScore = abs(armyInvestment		- techSchools[j].armyInvestment - 0.2) +
								abs(navyInvestment		- techSchools[j].navyInvestment - 0.2) +
								abs(commerceInvestment	- techSchools[j].commerceInvestment - 0.2) +
								abs(industryInvestment	- techSchools[j].industryInvestment - 0.2) +
								abs(cultureInvestment	- techSchools[j].cultureInvestment - 0.2);
		if (newScore < lowestScore)
		{
			bestSchool	= techSchools[j].name;
			lowestScore	= newScore;
		}
	}
	LOG(LogLevel::Debug) << tag << " has tech school " << bestSchool;
	techSchool = bestSchool;

	// Leaders
	vector<EU3Leader*> oldLeaders = srcCountry->getLeaders();
	for (vector<EU3Leader*>::iterator itr = oldLeaders.begin(); itr != oldLeaders.end(); ++itr)
	{
		V2Leader* leader = new V2Leader(*itr, lt);
		leaders.push_back(leader);
	}
}


// used only for countries which are NOT converted (i.e. unions, dead countries, etc)
void V2Country::initFromHistory()
{
	string fullFilename;
	struct _finddata_t	fileData;
	intptr_t					fileListing;
	string filesearch = ".\\blankMod\\output\\history\\countries\\" + tag + "*.txt";
	if ((fileListing = _findfirst(filesearch.c_str(), &fileData)) != -1L)
	{
		filename = fileData.name;
		fullFilename = string(".\\blankMod\\output\\history\\countries\\") + fileData.name;
	}
	_findclose(fileListing);
	if (fullFilename == "")
	{
		string filesearch = Configuration::getV2Path() + "\\history\\countries\\" + tag + "*.txt";
		if ((fileListing = _findfirst(filesearch.c_str(), &fileData)) != -1L)
		{
			filename = fileData.name;
			fullFilename = Configuration::getV2Path() + "\\history\\countries\\" + fileData.name;
		}
		_findclose(fileListing);
	}
	if (fullFilename == "")
	{
		string countryName = commonCountryFile;
		int lastSlash = countryName.find_last_of("/");
		countryName = countryName.substr(lastSlash + 1, countryName.size());
		filename = tag + " - " + countryName;
		return;
	}

	Object* obj = doParseFile(fullFilename.c_str());
	if (obj == NULL)
	{
		LOG(LogLevel::Error) << "Could not parse file " << fullFilename;
		exit(-1);
	}

	vector<Object*> results = obj->getValue("primary_culture");
	if (results.size() > 0)
	{
		primaryCulture = results[0]->getLeaf();
	}

	results = obj->getValue("culture");
	for (vector<Object*>::iterator itr = results.begin(); itr != results.end(); ++itr)
	{
		acceptedCultures.insert((*itr)->getLeaf());
	}

	results = obj->getValue("religion");
	if (results.size() > 0)
	{
		religion = results[0]->getLeaf();
	}

	results = obj->getValue("government");
	if (results.size() > 0)
	{
		government = results[0]->getLeaf();
	}

	results = obj->getValue("civilized");
	if (results.size() > 0)
	{
		civilized = (results[0]->getLeaf() == "yes");
	}

	results = obj->getValue("nationalvalue");
	if (results.size() > 0)
		nationalValue = results[0]->getLeaf();
	else
		nationalValue = "nv_order";

	results = obj->getValue("capital");
	if (results.size() > 0)
	{
		capital = atoi(results[0]->getLeaf().c_str());
	}
}


void V2Country::addProvince(V2Province* _province)
{
	auto itr = provinces.find(_province->getNum());
	if (itr != provinces.end())
	{
		LOG(LogLevel::Error) << "Inserting province " << _province->getNum() << " multiple times (addProvince())";
	}
	provinces.insert(make_pair(_province->getNum(), _province));
}


void V2Country::addState(V2State* newState)
{
	int				highestNavalLevel = 0;
	unsigned int	hasHighestLevel	= -1;
	bool				hasNavalBase		= false;

	states.push_back(newState);
	vector<V2Province*> newProvinces = newState->getProvinces();
	for (unsigned int i = 0; i < newProvinces.size(); i++)
	{
		auto itr = provinces.find(newProvinces[i]->getNum());
		if (itr == provinces.end())
		{
			provinces.insert(make_pair(newProvinces[i]->getNum(), newProvinces[i]));
		}

		// find the province with the highest naval base level
		if ((Configuration::getV2Gametype() == "HOD") || (Configuration::getV2Gametype() == "HoD-NNM"))
		{
			int navalLevel = 0;
			const EU3Province* srcProvince = newProvinces[i]->getSrcProvince();
			if (srcProvince != NULL)
			{
				if (srcProvince->hasBuilding("shipyard"))
				{
					navalLevel += 1;
				}
				if (srcProvince->hasBuilding("grand_shipyard"))
				{
					navalLevel += 1;
				}
				if (srcProvince->hasBuilding("naval_arsenal"))
				{
					navalLevel += 1;
				}
				if (srcProvince->hasBuilding("naval_base"))
				{
					navalLevel += 1;
				}
			}
			if (navalLevel > highestNavalLevel)
			{
				highestNavalLevel	= navalLevel;
				hasHighestLevel	= i;
			}
			newProvinces[i]->setNavalBaseLevel(0);
		}
	}

	if (( (Configuration::getV2Gametype() == "HOD") || (Configuration::getV2Gametype() == "HoD-NNM") ) && (highestNavalLevel > 0))
	{
		newProvinces[hasHighestLevel]->setNavalBaseLevel(1);
	}
}


//#define TEST_V2_PROVINCES
void V2Country::convertArmies(const map<int,int>& leaderIDMap, double cost_per_regiment[num_reg_categories], const inverseProvinceMapping& inverseProvinceMap, map<int, V2Province*> allProvinces, vector<int> port_whitelist, adjacencyMapping adjacencyMap)
{
#ifndef TEST_V2_PROVINCES
	if (srcCountry == NULL)
	{
 		return;
	}
	if (provinces.size() == 0)
	{
		return;
	}

	// set up armies with whatever regiments they deserve, rounded down
	// and keep track of the remainders for later
	double countryRemainder[num_reg_categories] = { 0.0 };
	vector<EU3Army*> sourceArmies = srcCountry->getArmies();
	for (vector<EU3Army*>::iterator aitr = sourceArmies.begin(); aitr != sourceArmies.end(); ++aitr)
	{
		V2Army* army = new V2Army(*aitr, leaderIDMap);

		for (int rc = infantry; rc < num_reg_categories; ++rc)
		{
			int typeStrength = (*aitr)->getTotalTypeStrength((RegimentCategory)rc);
			if (typeStrength == 0) // no regiments of this type
				continue;

			// if we have ships, we must be a navy
			bool isNavy = (rc >= big_ship); 
			army->setNavy(isNavy);

			double	regimentCount		= typeStrength / cost_per_regiment[rc];
			int		regimentsToCreate	= (int)floor(regimentCount);
			double	regimentRemainder	= regimentCount - regimentsToCreate;
			countryRemainder[rc] += regimentRemainder;
			army->setArmyRemainders((RegimentCategory)rc, army->getArmyRemainder((RegimentCategory)rc) + regimentRemainder);

			for (int i = 0; i < regimentsToCreate; ++i)
			{
				if (addRegimentToArmy(army, (RegimentCategory)rc, inverseProvinceMap, allProvinces, adjacencyMap) != 0)
				{
					// couldn't add, dissolve into pool
					countryRemainder[rc] += 1.0;
					army->setArmyRemainders((RegimentCategory)rc, army->getArmyRemainder((RegimentCategory)rc) + 1.0);
				}
			}
		}

		vector<int> locationCandidates = getV2ProvinceNums(inverseProvinceMap, (*aitr)->getLocation());
		if (locationCandidates.size() == 0)
		{
			LOG(LogLevel::Warning) << "Army or Navy " << (*aitr)->getName() << " assigned to unmapped province " << (*aitr)->getLocation() << "; dissolving to pool";
			int regimentCounts[num_reg_categories] = { 0 };
			army->getRegimentCounts(regimentCounts);
			for (int rc = infantry; rc < num_reg_categories; ++rc)
			{
				countryRemainder[rc] += regimentCounts[rc];
			}
			continue;
		}
		else if ((locationCandidates.size() == 1) && (locationCandidates[0] == 0))
		{
			LOG(LogLevel::Warning) << "Army or Navy " << (*aitr)->getName() << " assigned to dropped province " << (*aitr)->getLocation() << "; dissolving to pool";
			int regimentCounts[num_reg_categories] = { 0 };
			army->getRegimentCounts(regimentCounts);
			for (int rc = infantry; rc < num_reg_categories; ++rc)
			{
				countryRemainder[rc] += regimentCounts[rc];
			}
			continue;
		}
		bool usePort = false;
		// guarantee that navies are assigned to sea provinces, or land provinces with naval bases
		if (army->getNavy())
		{
			map<int, V2Province*>::iterator pitr = allProvinces.find(locationCandidates[0]);
			if (pitr != allProvinces.end())
			{
				usePort = true;
			}
			if (usePort)
			{
				locationCandidates = getPortProvinces(locationCandidates, allProvinces);
				if (locationCandidates.size() == 0)
				{
					LOG(LogLevel::Warning) << "Navy " << (*aitr)->getName() << " assigned to EU3 province " << (*aitr)->getLocation() << " which has no corresponding V2 port provinces; dissolving to pool";
					int regimentCounts[num_reg_categories] = { 0 };
					army->getRegimentCounts(regimentCounts);
					for (int rc = infantry; rc < num_reg_categories; ++rc)
					{
						countryRemainder[rc] += regimentCounts[rc];
					}
					continue;
				}
			}
		}
		int selectedLocation = locationCandidates[int(locationCandidates.size() * ((double)rand() / RAND_MAX))];
		if (army->getNavy() && usePort)
		{
			vector<int>::iterator white = std::find(port_whitelist.begin(), port_whitelist.end(), selectedLocation);
			if (white == port_whitelist.end())
			{
				LOG(LogLevel::Warning) << "Assigning navy to non-whitelisted port province " << selectedLocation << " - if the save crashes, try blacklisting this province";
			}
		}
		army->setLocation(selectedLocation);
		armies.push_back(army);
	}

	// allocate the remainders from the whole country to the armies according to their need, rounding up
	for (int rc = infantry; rc < num_reg_categories; ++rc)
	{
		while (countryRemainder[rc] > 0.0)
		{
			V2Army* army = getArmyForRemainder((RegimentCategory)rc);
			if (!army)
			{
				LOG(LogLevel::Debug) << "No suitable army or navy found for " << tag << "'s pooled regiments of " << RegimentCategoryNames[rc];
				break;
			}
			switch (addRegimentToArmy(army, (RegimentCategory)rc, inverseProvinceMap, allProvinces, adjacencyMap))
			{
			case 0: // success
				countryRemainder[rc] -= 1.0;
				army->setArmyRemainders((RegimentCategory)rc, army->getArmyRemainder((RegimentCategory)rc) - 1.0);
				break;
			case -1: // retry
				break;
			case -2: // do not retry
				LOG(LogLevel::Debug) << "Disqualifying army/navy " << army->getName() << " from receiving more " << RegimentCategoryNames[rc] << " from the pool";
				army->setArmyRemainders((RegimentCategory)rc, -2000.0);
				break;
			}
		}
	}

#else // ifdef TEST_V2_PROVINCES
	// output one big ship to each V2 province that's neither whitelisted nor blacklisted, but only 10 at a time per nation
	// output from this mode is used to build whitelist and blacklist files
	int n_tests = 0;
	for (vector<V2Province>::iterator pitr = provinces.begin(); (pitr != provinces.end()) && (n_tests < 50); ++pitr)
	{
		if ((pitr->getOwner() == itr->getTag()) && pitr->isCoastal())
		{
			vector<int>::iterator black = std::find(port_blacklist.begin(), port_blacklist.end(), pitr->getNum());
			if (black != port_blacklist.end())
				continue;

			V2Army army;
			army.setName("V2 Test Navy");
			army.setAtSea(0);
			army.setNavy(true);
			army.setLocation(pitr->getNum());
			V2Regiment reg(heavy_ship);
			reg.setStrength(100);
			army.addRegiment(reg);
			itr->addArmy(army);

			vector<int>::iterator white = std::find(port_whitelist.begin(), port_whitelist.end(), pitr->getNum());
			if (white == port_whitelist.end())
			{
				++n_tests;
				ofstream s("port_greylist.txt", ios_base::app);
				s << pitr->getNum() << "\n";
				s.close();
			}
		}
	}
	log("Output %d test ships.\n", n_tests);
#endif
}


void V2Country::getNationalValueScores(int& libertyScore, int& equalityScore, int& orderScore)
{
	orderScore += srcCountry->getOffensiveDefensive();
	orderScore += srcCountry->getInnovativeNarrowminded();
	orderScore += srcCountry->getQualityQuantity();
	if ( srcCountry->hasNationalIdea("deus_vult") )
	{
		orderScore += 2;
	}
	if ( srcCountry->hasNationalIdea("church_attendance_duty") )
	{
		orderScore += 2;
	}
	if ( srcCountry->hasNationalIdea("divine_supremacy") )
	{
		orderScore += 2;
	}
	if ( srcCountry->hasNationalIdea("national_conscripts") )
	{
		orderScore += 2;
	}
	if ( srcCountry->hasNationalIdea("press_gangs") )
	{
		orderScore += 1;
	}
	if ( srcCountry->hasNationalIdea("military_drill") )
	{
		orderScore += 1;
	}
	if ( srcCountry->hasNationalIdea("bureaucracy") )
	{
		orderScore += 1;
	}

	libertyScore += srcCountry->getCentralizationDecentralization();
	libertyScore += srcCountry->getSerfdomFreesubjects();
	libertyScore += srcCountry->getMercantilismFreetrade();
	if ( srcCountry->hasNationalIdea("liberty_egalite_fraternity") )
	{
		libertyScore += 4;
	}
	if ( srcCountry->hasNationalIdea("smithian_economics") )
	{
		libertyScore += 2;
	}
	if ( srcCountry->hasNationalIdea("bill_of_rights") )
	{
		libertyScore += 2;
	}
	if ( srcCountry->hasNationalIdea("scientific_revolution") )
	{
		libertyScore += 1;
	}
	if ( srcCountry->hasNationalIdea("ecumenism") )
	{
		libertyScore += 1;
	}

	equalityScore += srcCountry->getAristocracyPlutocracy();
	equalityScore += srcCountry->getSerfdomFreesubjects();
	if ( srcCountry->hasNationalIdea("liberty_egalite_fraternity") )
	{
		equalityScore += 4;
	}
	if ( srcCountry->hasNationalIdea("humanist_tolerance") )
	{
		equalityScore += 2;
	}
	if ( srcCountry->hasNationalIdea("bill_of_rights") )
	{
		equalityScore += 2;
	}
	if ( srcCountry->hasNationalIdea("ecumenism") )
	{
		equalityScore += 1;
	}
}


void V2Country::addRelation(V2Relations* newRelation)
{
	relations.insert(make_pair(newRelation->getTag(), newRelation));
}


static bool FactoryCandidateSortPredicate(const pair<double, V2State*>& lhs, const pair<double, V2State*>& rhs)
{
	if (lhs.first != rhs.first)
	{
		return lhs.first > rhs.first;
	}
	return lhs.second->getID() < rhs.second->getID();
}


bool V2Country::addFactory(V2Factory* factory)
{
	// check factory techs
	string requiredTech = factory->getRequiredTech();
	if (requiredTech != "")
	{
		vector<string>::iterator itr = find(techs.begin(), techs.end(), requiredTech);
		if (itr == techs.end())
		{
			LOG(LogLevel::Debug) << tag << " rejected " << factory->getTypeName() << " (missing required tech: " << requiredTech << ')';
			return false;
		}
	}
	
	// check factory inventions
	if ((Configuration::getV2Gametype() == "vanilla") || (Configuration::getV2Gametype() == "AHD"))
	{
		vanillaInventionType requiredInvention = factory->getVanillaRequiredInvention();
		if (requiredInvention >= 0 && vanillaInventions[requiredInvention] != active)
		{
			LOG(LogLevel::Debug) << tag << " rejected " << factory->getTypeName() << " (missing required invention: " << vanillaInventionNames[requiredInvention] << ')';
			return false;
		}
	}
	else if (Configuration::getV2Gametype() == "HOD")
	{
		HODInventionType requiredInvention = factory->getHODRequiredInvention();
		if (requiredInvention >= 0 && HODInventions[requiredInvention] != active)
		{
			LOG(LogLevel::Debug) << tag << " rejected " << factory->getTypeName() << " (missing required invention: " << HODInventionNames[requiredInvention] << ')';
			return false;
		}
	}
	else if (Configuration::getV2Gametype() == "HoD-NNM")
	{
		HODNNMInventionType requiredInvention = factory->getHODNNMRequiredInvention();
		if (requiredInvention >= 0 && HODNNMInventions[requiredInvention] != active)
		{
			LOG(LogLevel::Debug) << tag << " rejected " << factory->getTypeName() << " (missing reqd invention: " << HODNNMInventionNames[requiredInvention] << ')';
			return false;
		}
	}

	// find a state to add the factory to, which meets the factory's requirements
	vector<pair<double, V2State*>> candidates;
	for (vector<V2State*>::iterator itr = states.begin(); itr != states.end(); ++itr)
	{
		if ( (*itr)->isColonial() )
		{
			continue;
		}

		if ( (*itr)->getFactoryCount() >= 8 )
		{
			continue;
		}

		if (factory->requiresCoastal())
		{
			if ( !(*itr)->isCoastal() )
				continue;
		}

		if ( !(*itr)->hasLandConnection() )
		{
			continue;
		}

		map<string,float> requiredProducts = factory->getRequiredRGO();
		if (requiredProducts.size() > 0)
		{
			bool hasInput = false;
			for (map<string,float>::iterator prod = requiredProducts.begin(); prod != requiredProducts.end(); ++prod)
			{
				if ( (*itr)->hasLocalSupply(prod->first) )
				{
					hasInput = true;
					break;
				}
			}
			if (!hasInput)
				continue;
		}

		double candidateScore	 = (*itr)->getSuppliedInputs(factory) * 100;
		candidateScore				-= (*itr)->getFactoryCount() * 10;
		candidateScore				+= (*itr)->getManuRatio();
		candidates.push_back(pair<double, V2State*>(candidateScore, (*itr) ));
	}

	sort(candidates.begin(), candidates.end(), FactoryCandidateSortPredicate);

	if (candidates.size() == 0)
	{
		LOG(LogLevel::Debug) << tag << " rejected " << factory->getTypeName() << " (no candidate states)";
		return false;
	}

	V2State* target = candidates[0].second;
	target->addFactory(factory);
	LOG(LogLevel::Debug) << tag << " accepted " << factory->getTypeName() << " (" << candidates.size() << " candidate states)";
	numFactories++;
	return true;
}


void V2Country::addRailroadtoCapitalState()
{
	for (vector<V2State*>::iterator i = states.begin(); i != states.end(); i++)
	{
		if ( (*i)->provInState(capital) )
		{
			(*i)->addRailroads();
		}
	}
}


void V2Country::convertUncivReforms()
{
	if ((srcCountry != NULL) && ((Configuration::getV2Gametype() == "AHD") || (Configuration::getV2Gametype() == "HOD") || (Configuration::getV2Gametype() == "HoD-NNM")))
	{
		if (	(srcCountry->getTechGroup() == "western") || (srcCountry->getTechGroup() == "latin") ||
				(srcCountry->getTechGroup() == "eastern") || (srcCountry->getTechGroup() == "ottoman"))
		{
			// civilized, do nothing
		}
		else if ( (srcCountry->getTechGroup() == "nomad_group") || (srcCountry->getTechGroup() == "sub_saharan") || (srcCountry->getTechGroup() == "new_world") )
		{
			double totalTechs			= srcCountry->getLandTech() + srcCountry->getNavalTech() + srcCountry->getGovernmentTech() + 
											  srcCountry->getTradeTech() + srcCountry->getProductionTech();
			double militaryDev		= ( srcCountry->getLandTech() + srcCountry->getNavalTech() ) / totalTechs;
			double socioEconDev		= ( srcCountry->getGovernmentTech() + srcCountry->getTradeTech() + srcCountry->getProductionTech() ) / totalTechs;
			LOG(LogLevel::Debug) << "Setting unciv reforms for " << tag << " - westernization at 0%";
			uncivReforms	= new V2UncivReforms(0, militaryDev, socioEconDev, this);
			government		= "absolute_monarchy";
		}
		else if ( (srcCountry->getTechGroup() == "indian") || (srcCountry->getTechGroup() == "chinese") )
		{
			double totalTechs			= srcCountry->getLandTech() + srcCountry->getNavalTech() + srcCountry->getGovernmentTech() + 
											  srcCountry->getTradeTech() + srcCountry->getProductionTech();
			double militaryDev		= (srcCountry->getLandTech() + srcCountry->getNavalTech() ) / totalTechs;
			double socioEconDev		= (srcCountry->getGovernmentTech() + srcCountry->getTradeTech() + srcCountry->getProductionTech() ) / totalTechs;
			LOG(LogLevel::Debug) << "Setting unciv reforms for " << tag << " - westernization at 30%";
			uncivReforms	= new V2UncivReforms(30, militaryDev, socioEconDev, this);
			government		= "absolute_monarchy";
		}
		else if (srcCountry->getTechGroup() == "muslim")
		{
			double totalTechs			= srcCountry->getLandTech() + srcCountry->getNavalTech() + srcCountry->getGovernmentTech() + 
											  srcCountry->getTradeTech() + srcCountry->getProductionTech();
			double militaryDev		= ( srcCountry->getLandTech() + srcCountry->getNavalTech() ) / totalTechs;
			double socioEconDev		= ( srcCountry->getGovernmentTech() + srcCountry->getTradeTech() + srcCountry->getProductionTech() ) / totalTechs;
			LOG(LogLevel::Debug) << "Setting unciv reforms for " << tag << " - westernization at 60%";
			uncivReforms	= new V2UncivReforms(60, militaryDev, socioEconDev, this);
			government		= "absolute_monarchy";
		}
		else
		{
			LOG(LogLevel::Warning) << "Unhandled tech group (" << srcCountry->getTechGroup() << ") for " << tag << " - giving no reforms";
			double totalTechs			= srcCountry->getLandTech() + srcCountry->getNavalTech() + srcCountry->getGovernmentTech() + 
											  srcCountry->getTradeTech() + srcCountry->getProductionTech();
			double militaryDev		= ( srcCountry->getLandTech() + srcCountry->getNavalTech() ) / totalTechs;
			double socioEconDev		= ( srcCountry->getGovernmentTech() + srcCountry->getTradeTech() + srcCountry->getProductionTech() ) / totalTechs;
			uncivReforms	= new V2UncivReforms(0, militaryDev, socioEconDev, this);
			government		= "absolute_monarchy";
		}
	}
}


void V2Country::setupPops(EU3World& sourceWorld, double popWeightRatio)
{
	if (states.size() < 1) // skip entirely for empty nations
		return;

	// create the pops
	for (auto itr = provinces.begin(); itr != provinces.end(); ++itr)
	{
		itr->second->doCreatePops(sourceWorld.getWorldType(), popWeightRatio, this);
	}

	// output statistics on pops
	map<string, long int> popsData;
	for (auto provItr = provinces.begin(); provItr != provinces.end(); provItr++)
	{
		auto pops = provItr->second->getPops();
		for (auto popsItr = pops.begin(); popsItr != pops.end(); popsItr++)
		{
			auto popItr = popsData.find( (*popsItr)->getType() );
			if (popItr == popsData.end())
			{
				long int newPopSize = 0;
				pair<map<string, long int>::iterator, bool> newIterator = popsData.insert(make_pair((*popsItr)->getType(), 0));
				popItr = newIterator.first;
			}
			popItr->second += (*popsItr)->getSize();
		}
	}
	long int totalPops = 0;
	for (auto dataItr = popsData.begin(); dataItr != popsData.end(); dataItr++)
	{
		totalPops += dataItr->second;
	}

	/*for (auto dataItr = popsData.begin(); dataItr != popsData.end(); dataItr++)
	{
		double popsPercent = static_cast<double>(dataItr->second) / totalPops;
		string filename = dataItr->first;
		filename += ".csv";
		FILE* dataFile = fopen(filename.c_str(), "a");
		if (dataFile != NULL)
		{
			fprintf(dataFile, "%s,%d,%f\n", tag.c_str(), dataItr->second, popsPercent);
			fclose(dataFile);
		}
	}*/
}


void V2Country::setArmyTech(double mean, double highest)
{
	if (srcCountry == NULL)
	{
		return;
	}

	double newTechLevel = (srcCountry->getLandTech() - mean) / (highest - mean);
	LOG(LogLevel::Debug) << tag << " has army tech of " << newTechLevel;

	if ( (Configuration::getV2Gametype() == "vanilla") || (civilized == true) )
	{
		if (newTechLevel >= -1.0)
		{
			techs.push_back("flintlock_rifles");
			HODInventions[HOD_flintlock_rifle_armament] = active;
			HODNNMInventions[HOD_NNM_flintlock_rifle_armament] = active;
		}
		if (newTechLevel >= -0.9)
		{
			techs.push_back("bronze_muzzle_loaded_artillery");
		}
		if (newTechLevel >= -0.2)
		{
			techs.push_back("post_napoleonic_thought");
			HODInventions[HOD_post_napoleonic_army_doctrine]			= active;
			HODNNMInventions[HOD_NNM_post_napoleonic_army_doctrine]	= active;
		}
		if (newTechLevel >= 0.2)
		{
			techs.push_back("army_command_principle");
		}
		if (newTechLevel >= 0.6)
		{
			techs.push_back("military_staff_system");
			HODInventions[HOD_cuirassier_activation]			= active;
			HODInventions[HOD_dragoon_activation]				= active;
			HODInventions[HOD_hussar_activation]				= active;
			HODNNMInventions[HOD_NNM_cuirassier_activation]	= active;
			HODNNMInventions[HOD_NNM_dragoon_activation]		= active;
			HODNNMInventions[HOD_NNM_hussar_activation]		= active;
		}
		if (newTechLevel >= 1.0)
		{
			techs.push_back("army_professionalism");
			vanillaInventions[VANILLA_army_academic_training]	= active;
			vanillaInventions[VANILLA_field_training]				= active;
			vanillaInventions[VANILLA_army_societal_status]		= active;
			HODInventions[HOD_army_academic_training]				= active;
			HODInventions[HOD_field_training]						= active;
			HODInventions[HOD_army_societal_status]				= active;
			HODNNMInventions[HOD_NNM_army_academic_training]	= active;
			HODNNMInventions[HOD_NNM_field_training]				= active;
			HODNNMInventions[HOD_NNM_army_societal_status]		= active;
		}
	}
}


void V2Country::setNavyTech(double mean, double highest)
{
	if (srcCountry == NULL)
	{
		return;
	}

	double newTechLevel = (srcCountry->getNavalTech() - mean) / (highest - mean);
	LOG(LogLevel::Debug) << tag << " has navy tech of " << newTechLevel;

	if ( (Configuration::getV2Gametype() == "vanilla") || (civilized == true) )
	{
		if (newTechLevel >= 0)
		{
			techs.push_back("post_nelsonian_thought");
			HODInventions[HOD_long_range_fire_tactic]					= active;
			HODInventions[HOD_speedy_maneuvering_tactic]				= active;
			HODNNMInventions[HOD_NNM_long_range_fire_tactic]		= active;
			HODNNMInventions[HOD_NNM_speedy_maneuvering_tactic]	= active;
		}
		if (newTechLevel >= 0.036)
		{
			techs.push_back("the_command_principle");
		}
		if (newTechLevel >= 0.571)
		{
			techs.push_back("clipper_design");
			techs.push_back("naval_design_bureaus");
			techs.push_back("alphabetic_flag_signaling");
			vanillaInventions[VANILLA_building_station_shipyards]	= active;
			HODInventions[HOD_building_station_shipyards]			= active;
			HODNNMInventions[HOD_NNM_building_station_shipyards]	= active;
		}
		if (newTechLevel >= 0.857)
		{
			techs.push_back("battleship_column_doctrine");
			techs.push_back("steamers");
			vanillaInventions[VANILLA_long_range_fire_tactic]						= active;
			HODInventions[HOD_long_range_fire_tactic]									= active;
			HODNNMInventions[HOD_NNM_long_range_fire_tactic]						= active;
			vanillaInventions[VANILLA_speedy_maneuvering_tactic]					= active;
			HODInventions[HOD_speedy_maneuvering_tactic]								= active;
			HODNNMInventions[HOD_NNM_speedy_maneuvering_tactic]					= active;
			vanillaInventions[VANILLA_mechanized_fishing_vessels]					= active;
			HODInventions[HOD_mechanized_fishing_vessels]							= active;
			HODNNMInventions[HOD_NNM_mechanized_fishing_vessels]					= active;
			vanillaInventions[VANILLA_steamer_automatic_construction_plants]	= active;
			HODInventions[HOD_steamer_automatic_construction_plants]				= active;
			HODNNMInventions[HOD_NNM_steamer_automatic_construction_plants]	= active;
			vanillaInventions[VANILLA_steamer_transports]							= active;
			HODInventions[HOD_steamer_transports]										= active;
			HODNNMInventions[HOD_NNM_steamer_transports]								= active;
			vanillaInventions[VANILLA_commerce_raiders]								= active;
			HODInventions[HOD_commerce_raiders]											= active;
			HODNNMInventions[HOD_NNM_commerce_raiders]								= active;
		}
		if (newTechLevel >= 1.0)
		{
			techs.push_back("naval_professionalism");
			vanillaInventions[VANILLA_academic_training]			= active;
			vanillaInventions[VANILLA_combat_station_training]	= active;
			vanillaInventions[VANILLA_societal_status]			= active;
			HODInventions[HOD_academic_training]					= active;
			HODInventions[HOD_combat_station_training]			= active;
			HODInventions[HOD_societal_status]						= active;
			HODNNMInventions[HOD_NNM_academic_training]			= active;
			HODNNMInventions[HOD_NNM_combat_station_training]	= active;
			HODNNMInventions[HOD_NNM_societal_status]				= active;
		}
	}
}


void V2Country::setCommerceTech(double mean, double highest)
{
	if (srcCountry == NULL)
	{
		return;
	}

	double newTechLevel = (srcCountry->getTradeTech() - mean) / (highest - mean);
	LOG(LogLevel::Debug) << tag << " has commerce tech of " << newTechLevel;

	if ( (Configuration::getV2Gametype() == "vanilla") || (civilized == true) )
	{
		techs.push_back("no_standard");
		if (newTechLevel >= -0.777)
		{
			techs.push_back("guild_based_production");
		}
		if (newTechLevel >= -0.555)
		{
			techs.push_back("private_banks");
		}
		if (newTechLevel >= -0.333)
		{
			techs.push_back("early_classical_theory_and_critique");
		}
		if (newTechLevel >= -0.277)
		{
			techs.push_back("freedom_of_trade");
			vanillaInventions[VANILLA_john_ramsay_mcculloch]	= active;
			HODInventions[HOD_john_ramsay_mcculloch]				= active;
			HODNNMInventions[HOD_NNM_john_ramsay_mcculloch]		= active;
			vanillaInventions[VANILLA_nassau_william_sr]			= active;
			HODInventions[HOD_nassau_william_sr]					= active;
			HODNNMInventions[HOD_NNM_nassau_william_sr]			= active;
			vanillaInventions[VANILLA_james_mill]					= active;
			HODInventions[HOD_james_mill]								= active;
			HODNNMInventions[HOD_NNM_james_mill]					= active;
		}
		if (newTechLevel >= 0.333)
		{
			techs.push_back("stock_exchange");
			vanillaInventions[VANILLA_multitude_of_financial_instruments]		= active;
			HODInventions[HOD_multitude_of_financial_instruments]					= active;
			HODNNMInventions[HOD_NNM_multitude_of_financial_instruments]		= active;
			vanillaInventions[VANILLA_insurance_companies]							= active;
			HODInventions[HOD_insurance_companies]										= active;
			HODNNMInventions[HOD_NNM_insurance_companies]							= active;
			vanillaInventions[VANILLA_regulated_buying_and_selling_of_stocks]	= active;
			HODInventions[HOD_regulated_buying_and_selling_of_stocks]			= active;
			HODNNMInventions[HOD_NNM_regulated_buying_and_selling_of_stocks]	= active;
		}
		if (newTechLevel >= 0.777)
		{
			techs.push_back("ad_hoc_money_bill_printing");
			techs.push_back("market_structure");
			vanillaInventions[VANILLA_silver_standard]			= active;
			HODInventions[HOD_silver_standard]						= active;
			HODNNMInventions[HOD_NNM_silver_standard]				= active;
			vanillaInventions[VANILLA_decimal_monetary_system]	= active;
			HODInventions[HOD_decimal_monetary_system]			= active;
			HODNNMInventions[HOD_NNM_decimal_monetary_system]	= active;
			vanillaInventions[VANILLA_polypoly_structure]		= active;
			HODInventions[HOD_polypoly_structure]					= active;
			HODNNMInventions[HOD_NNM_polypoly_structure]			= active;
			vanillaInventions[VANILLA_oligopoly_structure]		= active;
			HODInventions[HOD_oligopoly_structure]					= active;
			HODNNMInventions[HOD_NNM_oligopoly_structure]		= active;
			vanillaInventions[VANILLA_monopoly_structure]		= active;
			HODInventions[HOD_monopoly_structure]					= active;
			HODNNMInventions[HOD_NNM_monopoly_structure]			= active;
		}
		if (newTechLevel >= 1.0)
		{
			techs.push_back("late_classical_theory");
			vanillaInventions[VANILLA_john_elliot_cairnes]	= active;
			vanillaInventions[VANILLA_robert_torrens]			= active;
			vanillaInventions[VANILLA_john_stuart_mill]		= active;
			HODInventions[HOD_john_elliot_cairnes]				= active;
			HODInventions[HOD_robert_torrens]					= active;
			HODInventions[HOD_john_stuart_mill]					= active;
			HODNNMInventions[HOD_NNM_john_elliot_cairnes]	= active;
			HODNNMInventions[HOD_NNM_robert_torrens]			= active;
			HODNNMInventions[HOD_NNM_john_stuart_mill]		= active;
		}
	}
}


void V2Country::setIndustryTech(double mean, double highest)
{
	if (srcCountry == NULL)
	{
		return;
	}

	double newTechLevel = (srcCountry->getProductionTech() - mean) / (highest - mean);
	LOG(LogLevel::Debug) << tag << " has industry tech of " << newTechLevel;

	if ( (Configuration::getV2Gametype() == "vanilla") || (civilized == true) )
	{
		if (newTechLevel >= -1.0)
		{
			techs.push_back("water_wheel_power");
			HODInventions[HOD_tulls_seed_drill]	= active;
		}
		if (newTechLevel >= -0.714)
		{
			techs.push_back("publishing_industry");
		}
		if (newTechLevel >= -0.143)
		{
			techs.push_back("mechanized_mining");
			techs.push_back("basic_chemistry");
			vanillaInventions[VANILLA_ammunition_production]	= active;
			HODInventions[HOD_ammunition_production]				= active;
			HODNNMInventions[HOD_NNM_ammunition_production]		= active;
			vanillaInventions[VANILLA_small_arms_production]	= active;
			HODInventions[HOD_small_arms_production]				= active;
			HODNNMInventions[HOD_NNM_small_arms_production]		= active;
			vanillaInventions[VANILLA_explosives_production]	= active;
			HODInventions[HOD_explosives_production]				= active;
			HODNNMInventions[HOD_NNM_explosives_production]		= active;
			vanillaInventions[VANILLA_artillery_production]		= active;
			HODInventions[HOD_artillery_production]				= active;
			HODNNMInventions[HOD_NNM_artillery_production]		= active;
		}
		if (newTechLevel >= 0.143)
		{
			techs.push_back("practical_steam_engine");
			HODInventions[HOD_rotherham_plough]						= active;
			HODNNMInventions[HOD_NNM_rotherham_plough]			= active;
		}
		if (newTechLevel >= 0.428)
		{
			techs.push_back("experimental_railroad");
		}
		if (newTechLevel >= 0.714)
		{
			techs.push_back("mechanical_production");
			HODInventions[HOD_sharp_n_roberts_power_loom]						= active;
			HODNNMInventions[HOD_NNM_sharp_n_roberts_power_loom]				= active;
			vanillaInventions[VANILLA_sharp_n_roberts_power_loom]				= active;
			HODInventions[HOD_jacquard_power_loom]									= active;
			HODNNMInventions[HOD_NNM_jacquard_power_loom]						= active;
			vanillaInventions[VANILLA_jacquard_power_loom]						= active;
			HODInventions[HOD_northrop_power_loom]									= active;
			HODNNMInventions[HOD_NNM_northrop_power_loom]						= active;
			vanillaInventions[VANILLA_northrop_power_loom]						= active;
			HODInventions[HOD_mechanical_saw]										= active;
			HODNNMInventions[HOD_NNM_mechanical_saw]								= active;
			vanillaInventions[VANILLA_mechanical_saw]								= active;
			HODInventions[HOD_mechanical_precision_saw]							= active;
			HODNNMInventions[HOD_NNM_mechanical_precision_saw]					= active;
			vanillaInventions[VANILLA_mechanical_precision_saw]				= active;
			HODInventions[HOD_hussey_n_mccormicks_reaping_machine]			= active;
			HODNNMInventions[HOD_NNM_hussey_n_mccormicks_reaping_machine]	= active;
			vanillaInventions[VANILLA_hussey_n_mccormicks_reaping_machine]	= active;
			HODInventions[HOD_pitts_threshing_machine]							= active;
			HODNNMInventions[HOD_NNM_pitts_threshing_machine]					= active;
			vanillaInventions[VANILLA_pitts_threshing_machine]					= active;
			HODInventions[HOD_mechanized_slaughtering_block]					= active;
			HODNNMInventions[HOD_NNM_mechanized_slaughtering_block]			= active;
			vanillaInventions[VANILLA_mechanized_slaughtering_block]			= active;
			HODInventions[HOD_precision_work]										= active;
			HODNNMInventions[HOD_NNM_precision_work]								= active;
		}
		if (newTechLevel >= 1.0)
		{
			techs.push_back("clean_coal");
			vanillaInventions[VANILLA_pit_coal]	= active;
			vanillaInventions[VANILLA_coke]		= active;
			HODInventions[HOD_pit_coal]			= active;
			HODInventions[HOD_coke]					= active;
			HODNNMInventions[HOD_NNM_pit_coal]	= active;
			HODNNMInventions[HOD_NNM_coke]		= active;
		}
	}
}


void V2Country::setCultureTech(double mean, double highest)
{
	if (srcCountry == NULL)
	{
		return;
	}

	double newTechLevel = (srcCountry->getGovernmentTech() - mean) / (highest - mean);
	LOG(LogLevel::Debug) << tag << " has culture tech of " << newTechLevel;

	if ( (Configuration::getV2Gametype() == "vanilla") || (civilized == true) )
	{
		techs.push_back("classicism_n_early_romanticism");
		HODNNMInventions[HOD_NNM_carlism] = active;
		techs.push_back("late_enlightenment_philosophy");
		if (newTechLevel >= -0.333)
		{
			techs.push_back("enlightenment_thought");
			HODNNMInventions[HOD_NNM_declaration_of_the_rights_of_man]	= active;
			HODInventions[HOD_paternalism]			= active;
			HODNNMInventions[HOD_NNM_caste_privileges] = active;
			HODNNMInventions[HOD_NNM_sati_abolished] = active;
			HODInventions[HOD_constitutionalism]	= active;
			HODNNMInventions[HOD_NNM_pig_fat_cartridges] = active;
			HODInventions[HOD_atheism]					= active;
			HODInventions[HOD_egalitarianism]		= active;
			HODInventions[HOD_rationalism]			= active;
		}
		if (newTechLevel >= 0.333)
		{
			techs.push_back("malthusian_thought");
		}
		if (newTechLevel >= 0.333)
		{
			techs.push_back("introspectionism");
		}
		if (newTechLevel >= 0.666)
		{
			techs.push_back("romanticism");
			vanillaInventions[VANILLA_romanticist_literature]	= active;
			HODNNMInventions[HOD_NNM_romanticist_literature]	= active;
			HODNNMInventions[HOD_NNM_romanticist_literature]	= active;
			vanillaInventions[VANILLA_romanticist_art]			= active;
			HODNNMInventions[HOD_NNM_romanticist_art]				= active;
			HODNNMInventions[HOD_NNM_romanticist_art]				= active;
			vanillaInventions[VANILLA_romanticist_music]			= active;
			HODNNMInventions[HOD_NNM_romanticist_music]			= active;
			HODNNMInventions[HOD_NNM_romanticist_music]			= active;
		}
	}
}


V2Relations* V2Country::getRelations(string withWhom) const
{
	map<string, V2Relations*>::const_iterator i = relations.find(withWhom);
	if (i != relations.end())
	{
		return i->second;
	}
	else
	{
		return NULL;
	}
}


void V2Country::addLoan(string creditor, double size, double interest)
{
	map<string, V2Creditor*>::iterator itr = creditors.find(creditor);
	if (itr != creditors.end())
	{
			itr->second->addLoan(size, interest);
	}
	else
	{
		V2Creditor* cred = new V2Creditor(creditor);
		cred->addLoan(size, interest);
		creditors.insert(make_pair(creditor, cred));
	}
}


// return values: 0 = success, -1 = retry from pool, -2 = do not retry
int V2Country::addRegimentToArmy(V2Army* army, RegimentCategory rc, const inverseProvinceMapping& inverseProvinceMap, map<int, V2Province*> allProvinces, adjacencyMapping adjacencyMap)
{
	V2Regiment reg((RegimentCategory)rc);
	int eu3Home = army->getSourceArmy()->getProbabilisticHomeProvince(rc);
	if (eu3Home == -1)
	{
		LOG(LogLevel::Debug) << "Army/navy " << army->getName() << " has no valid home provinces for " << RegimentCategoryNames[rc] << " due to previous errors; dissolving to pool";
		return -2;
	}
	vector<int> homeCandidates = getV2ProvinceNums(inverseProvinceMap, eu3Home);
	if (homeCandidates.size() == 0)
	{
		LOG(LogLevel::Warning) << RegimentCategoryNames[rc] << " unit in army/navy " << army->getName() << " has unmapped home province " << eu3Home << " - dissolving to pool";
		army->getSourceArmy()->blockHomeProvince(eu3Home);
		return -1;
	}
	if (homeCandidates[0] == 0)
	{
		LOG(LogLevel::Warning) << RegimentCategoryNames[rc] << " unit in army/navy " << army->getName() << " has dropped home province " << eu3Home << " - dissolving to pool";
		army->getSourceArmy()->blockHomeProvince(eu3Home);
		return -1;
	}
	V2Province* homeProvince = NULL;
	if (army->getNavy())
 	{
		// Navies should only get homes in port provinces
		homeCandidates = getPortProvinces(homeCandidates, allProvinces);
		if (homeCandidates.size() != 0)
		{
			int homeProvinceID = homeCandidates[int(homeCandidates.size() * ((double)rand() / RAND_MAX))];
			map<int, V2Province*>::iterator pitr = allProvinces.find(homeProvinceID);
			if (pitr != allProvinces.end())
			{
				homeProvince = pitr->second;
			}
		}
	}
	else
	{
		// Armies should get a home in the candidate most capable of supporting them
		vector<V2Province*> sortedHomeCandidates;
		for (vector<int>::iterator nitr = homeCandidates.begin(); nitr != homeCandidates.end(); ++nitr)
		{
			map<int, V2Province*>::iterator pitr = allProvinces.find(*nitr);
			if (pitr != allProvinces.end())
			{
				sortedHomeCandidates.push_back(pitr->second);
			}
		}
		sort(sortedHomeCandidates.begin(), sortedHomeCandidates.end(), ProvinceRegimentCapacityPredicate);
		if (sortedHomeCandidates.size() == 0)
		{
			LOG(LogLevel::Warning) << "No valid home for a " << tag << RegimentCategoryNames[rc] << " regiment - dissolving regiment to pool";
			// all provinces in a given province map have the same owner, so the source home was bad
			army->getSourceArmy()->blockHomeProvince(eu3Home);
			return -1;
		}
		homeProvince = sortedHomeCandidates[0];
		if (homeProvince->getOwner() != tag)
		{
			map<int, V2Province*>	openProvinces = allProvinces;
			queue<int>					goodProvinces;

			map<int, V2Province*>::iterator openItr = openProvinces.find(homeProvince->getNum());
			homeProvince = NULL;
			if ( (openItr != openProvinces.end()) && (provinces.size() > 0) )
			{
				goodProvinces.push(openItr->first);
				openProvinces.erase(openItr);

				do
				{
					int currentProvince = goodProvinces.front();
					goodProvinces.pop();
					if (currentProvince > static_cast<int>(adjacencyMap.size()))
					{
						LOG(LogLevel::Warning) << "No adjacency mapping for province " << currentProvince;
						continue;
					}
					vector<int> adjacencies = adjacencyMap[currentProvince];
					for (unsigned int i = 0; i < adjacencies.size(); i++)
					{
						map<int, V2Province*>::iterator openItr = openProvinces.find(adjacencies[i]);
						if (openItr == openProvinces.end())
						{
							continue;
						}
						if (openItr->second->getOwner() == tag)
						{
							homeProvince = openItr->second;
						}
						goodProvinces.push(openItr->first);
						openProvinces.erase(openItr);
					}
				} while ((goodProvinces.size() > 0) && (homeProvince == NULL));
			}
			if (homeProvince == NULL)
			{
				LOG(LogLevel::Warning) << "V2 province " << sortedHomeCandidates[0]->getNum() << " is home for a " << tag << " " << RegimentCategoryNames[rc] << " regiment, but belongs to " << sortedHomeCandidates[0]->getOwner() << " - dissolving regiment to pool";
				// all provinces in a given province map have the same owner, so the source home was bad
				army->getSourceArmy()->blockHomeProvince(eu3Home);
				return -1;
			}
			return 0;
		}

		// Armies need to be associated with pops
		V2Pop* soldierPop = homeProvince->getSoldierPopForArmy();
		if (NULL == soldierPop)
		{
			// if the old home province was colonized and can't support the unit, try turning it into an "expeditionary" army
			if (homeProvince->wasColony())
			{
				V2Province* expSender = getProvinceForExpeditionaryArmy();
				if (expSender)
				{
					V2Pop* expSoldierPop = expSender->getSoldierPopForArmy();
					if (NULL != expSoldierPop)
					{
						homeProvince = expSender;
						soldierPop = expSoldierPop;
					}
				}
			}
		}
		if (NULL == soldierPop)
		{
			soldierPop = homeProvince->getSoldierPopForArmy(true);
		}
		reg.setHome(homeProvince->getNum());
	}
	if (homeProvince != NULL)
	{
		reg.setName(homeProvince->getRegimentName(rc));
	}
	else
	{
		reg.setName(getRegimentName(rc));
	}
	army->addRegiment(reg);
	return 0;
}


vector<int> V2Country::getPortProvinces(vector<int> locationCandidates, map<int, V2Province*> allProvinces)
{
	// hack for naval bases.  not ALL naval bases are in port provinces, and if you spawn a navy at a naval base in
	// a non-port province, Vicky crashes....
	static set<int> port_blacklist;
	if (port_blacklist.size() == 0)
	{
		int temp = 0;
		ifstream s("port_blacklist.txt");
		while (s.good() && !s.eof())
		{
			s >> temp;
			port_blacklist.insert(temp);
		}
		s.close();
	}

	vector<int> unblockedCandidates;
	for (vector<int>::iterator litr = locationCandidates.begin(); litr != locationCandidates.end(); ++litr)
	{
		auto black = port_blacklist.find(*litr);
		if (black == port_blacklist.end())
		{
			unblockedCandidates.push_back(*litr);
		}
	}
	locationCandidates.swap(unblockedCandidates);

	for (vector<int>::iterator litr = locationCandidates.begin(); litr != locationCandidates.end(); ++litr)
	{
		map<int, V2Province*>::iterator pitr = allProvinces.find(*litr);
		if (pitr != allProvinces.end())
		{
			if ( !pitr->second->isCoastal() )
			{
				locationCandidates.erase(litr);
				--pitr;
				break;
			}
		}
	}
	return locationCandidates;
}


// find the army most in need of a regiment of this category
V2Army*	V2Country::getArmyForRemainder(RegimentCategory rc)
{
	V2Army* retval = NULL;
	double retvalRemainder = -1000.0;
	for (vector<V2Army*>::iterator itr = armies.begin(); itr != armies.end(); ++itr)
	{
		// only add units to armies that originally had units of the same category
		if ( (*itr)->getSourceArmy()->getTotalTypeStrength(rc) > 0 )
		{
			if ( (*itr)->getArmyRemainder(rc) > retvalRemainder )
			{
				retvalRemainder = (*itr)->getArmyRemainder(rc);
				retval = *itr;
			}
		}
	}
	return retval;
}


bool ProvinceRegimentCapacityPredicate(V2Province* prov1, V2Province* prov2)
{
	return (prov1->getAvailableSoldierCapacity() > prov2->getAvailableSoldierCapacity());
}


V2Province* V2Country::getProvinceForExpeditionaryArmy()
{
	vector<V2Province*> candidates;
	for (auto pitr = provinces.begin(); pitr != provinces.end(); ++pitr)
	{
		if ( (pitr->second->getOwner() == tag) && !pitr->second->wasColony() && !pitr->second->wasInfidelConquest()
			&& ( pitr->second->hasCulture(primaryCulture, 0.5) ) && ( pitr->second->getPops("soldiers").size() > 0) )
 		{
			candidates.push_back(pitr->second);
 		}
	}
	if (candidates.size() > 0)
	{
		sort(candidates.begin(), candidates.end(), ProvinceRegimentCapacityPredicate);
		return candidates[0];
	}
	return NULL;
}


string V2Country::getRegimentName(RegimentCategory rc)
{
	// galleys turn into light ships; count and name them identically
	if (rc == galley)
		rc = light_ship;

	stringstream str;
	str << ++unitNameCount[rc] << CardinalToOrdinal(unitNameCount[rc]); // 1st, 2nd, etc
	string adjective = localisation.GetLocalAdjective();
	if (adjective == "")
	{
		str << " ";
	}
	else
	{
		str << " " << adjective << " ";
	}
	switch (rc)
	{
		case artillery:
			str << "Artillery";
			break;
		case infantry:
			str << "Infantry";
			break;
		case cavalry:
			str << "Cavalry";
			break;
		case big_ship:
			str << "Man'o'war";
			break;
		case light_ship:
			str << "Frigate";
			break;
		case transport:
			str << "Clipper Transport";
			break;
	}
	return str.str();
}