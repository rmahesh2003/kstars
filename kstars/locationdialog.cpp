/***************************************************************************
                          locationdialog.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Feb 11 2001
    copyright            : (C) 2001 by Jason Harris
    email                : jharris@30doradus.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <stdlib.h>
#include <stdio.h>

#include <QFile>
#include <QTextStream>

#include <kmessagebox.h>
#include <kstandarddirs.h>

#include "kstars.h"
#include "kstarsdata.h"

#include "locationdialog.h"

LocationDialogUI::LocationDialogUI( QWidget *parent ) : QFrame( parent ) {
	setupUi( this );
}

LocationDialog::LocationDialog( QWidget* parent )
    : KDialog( parent, i18n( "Set Geographic Location" ), Ok|Cancel ) {

	KStars *p = (KStars *)parent;

	ui = new LocationDialogUI( this );
	setMainWidget( ui );

	for ( int i=0; i<25; ++i )
		ui->TZBox->addItem( KGlobal::locale()->formatNumber( (double)(i-12) ) );

	//Populate DSTRuleBox
	QMap<QString, TimeZoneRule>::Iterator it = p->data()->Rulebook.begin();
	QMap<QString, TimeZoneRule>::Iterator itEnd = p->data()->Rulebook.end();
	for ( ; it != itEnd; ++it )
		if ( it.key().length() )
			ui->DSTRuleBox->addItem( it.key() );

	connect( this, SIGNAL( cancelClicked() ), this, SLOT( reject() ) );
	connect( ui->CityFilter, SIGNAL( textChanged( const QString & ) ), this, SLOT( filterCity() ) );
	connect( ui->ProvinceFilter, SIGNAL( textChanged( const QString & ) ), this, SLOT( filterCity() ) );
	connect( ui->CountryFilter, SIGNAL( textChanged( const QString & ) ), this, SLOT( filterCity() ) );
	connect( ui->NewCityName, SIGNAL( textChanged( const QString & ) ), this, SLOT( nameChanged() ) );
	connect( ui->NewProvinceName, SIGNAL( textChanged( const QString & ) ), this, SLOT( nameChanged() ) );
	connect( ui->NewCountryName, SIGNAL( textChanged( const QString & ) ), this, SLOT( nameChanged() ) );
	connect( ui->NewLong, SIGNAL( textChanged( const QString & ) ), this, SLOT( dataChanged() ) );
	connect( ui->NewLat, SIGNAL( textChanged( const QString & ) ), this, SLOT( dataChanged() ) );
	connect( ui->TZBox, SIGNAL( activated(int) ), this, SLOT( dataChanged() ) );
	connect( ui->DSTRuleBox, SIGNAL( activated(int) ), this, SLOT( dataChanged() ) );
	connect( ui->GeoBox, SIGNAL( selectionChanged() ), this, SLOT( changeCity() ) );
	connect( ui->AddCityButton, SIGNAL( clicked() ), this, SLOT( addCity() ) );
	connect( ui->ClearFieldsButton, SIGNAL( clicked() ), this, SLOT( clearFields() ) );

	disconnect( ui->DSTLabel, SIGNAL( linkClicked(const QString &) ), ui->DSTLabel, SLOT(openLink(const QString &) ) );
	connect( ui->DSTLabel, SIGNAL( linkClicked(const QString &) ), this, SLOT( showTZRules() ) );

	dataModified = false;
	nameModified = false;
	ui->AddCityButton->setEnabled( false );

	ui->NewCityName->setTrapReturnKey(true);
	ui->NewProvinceName->setTrapReturnKey(true);
	ui->NewCountryName->setTrapReturnKey(true);
	ui->CityFilter->setTrapReturnKey(true);
	ui->ProvinceFilter->setTrapReturnKey(true);
	ui->CountryFilter->setTrapReturnKey(true);

	initCityList();
	resize (640, 480);
}

//Do NOT delete members of filteredCityList! They are not created by LocationDialog
LocationDialog::~LocationDialog(){
}

void LocationDialog::initCityList( void ) {
	KStars *p = (KStars *)parent();
	foreach ( GeoLocation *loc, p->data()->geoList )
	{
		ui->GeoBox->insertItem( loc->fullName() );
		filteredCityList.append( loc );

		//If TZ is not an even integer value, add it to listbox
		if ( loc->TZ0() - int( loc->TZ0() ) && ui->TZBox->findText( KGlobal::locale()->formatNumber( loc->TZ0() ) ) != -1 ) {
			for ( int i=0; i < ui->TZBox->count(); ++i ) {
				if ( ui->TZBox->text( i ).toDouble() > loc->TZ0() ) {
					ui->TZBox->addItem( KGlobal::locale()->formatNumber( loc->TZ0() ), i-1 );
					break;
				}
			}
		}
	}

	//Sort the list of Cities alphabetically...note that filteredCityList may now have a different ordering!
	ui->GeoBox->sort();
	
	ui->CountLabel->setText( i18np("One city matches search criteria","%n cities match search criteria", ui->GeoBox->count()) );

	// attempt to highlight the current kstars location in the GeoBox
	ui->GeoBox->setCurrentItem( 0 );
	if ( ui->GeoBox->count() ) {
		for ( uint i=0; i < ui->GeoBox->count(); i++ ) {
			if ( ui->GeoBox->item(i)->text() == p->geo()->fullName() ) {
				ui->GeoBox->setCurrentItem( i );
				break;
			}
		}
	}
}

void LocationDialog::filterCity( void ) {
	KStars *p = (KStars *)parent();
	ui->GeoBox->clear();
	//Do NOT delete members of filteredCityList!
	while ( ! filteredCityList.isEmpty() ) filteredCityList.takeFirst();
	
	nameModified = false;
	dataModified = false;
	ui->AddCityButton->setEnabled( false );

	foreach ( GeoLocation *loc, p->data()->geoList ) {
		QString sc( loc->translatedName() );
		QString ss( loc->translatedCountry() );
		QString sp = "";
		if ( !loc->province().isEmpty() )
			sp = loc->translatedProvince();

		if ( sc.toLower().startsWith( ui->CityFilter->text().toLower() ) &&
				sp.toLower().startsWith( ui->ProvinceFilter->text().toLower() ) &&
				ss.toLower().startsWith( ui->CountryFilter->text().toLower() ) ) {

			ui->GeoBox->insertItem( loc->fullName() );
			filteredCityList.append( loc );
		}
	}

	ui->GeoBox->sort();
	
	ui->CountLabel->setText( i18np("One city matches search criteria","%n cities match search criteria", ui->GeoBox->count()) );

	if ( ui->GeoBox->firstItem() )		// set first item in list as selected
		ui->GeoBox->setCurrentItem( ui->GeoBox->firstItem() );

	ui->MapView->repaint();
}

void LocationDialog::changeCity( void ) {
	//when the selected city changes, set newCity, and redraw map
	SelectedCity = 0L;
	if ( ui->GeoBox->currentItem() >= 0 ) {
		for ( int i=0; i < filteredCityList.size(); ++i ) {
			GeoLocation *loc = filteredCityList.at(i);
			if ( loc->fullName() == ui->GeoBox->currentText() ) {
				SelectedCity = loc;
				break;
			}
		}
	}
	
	ui->MapView->repaint();

	//Fill the fields at the bottom of the window with the selected city's data.
	if ( SelectedCity ) {
		KStars *p = (KStars *)parent();
		ui->NewCityName->setText( SelectedCity->translatedName() );
		ui->NewProvinceName->setText( SelectedCity->translatedProvince() );
		ui->NewCountryName->setText( SelectedCity->translatedCountry() );
		ui->NewLong->showInDegrees( SelectedCity->lng() );
		ui->NewLat->showInDegrees( SelectedCity->lat() );
		ui->TZBox->setEditText( KGlobal::locale()->formatNumber( SelectedCity->TZ0() ) );
		
		//Pick the City's rule from the rulebook
		for ( int i=0; i < ui->DSTRuleBox->count(); ++i ) {
			TimeZoneRule tzr = p->data()->Rulebook.value( ui->DSTRuleBox->text(i) );
			if ( tzr.equals( SelectedCity->tzrule() ) ) {
				ui->DSTRuleBox->setCurrentItem( i );
				break;
			}
		}
	}
	
	nameModified = false;
	dataModified = false;
	ui->AddCityButton->setEnabled( false );
}

void LocationDialog::addCity( void ) {
	KStars *p = (KStars *)parent();
	bCityAdded = false;

	if ( !nameModified && !dataModified ) {
		QString message = i18n( "This City already exists in the database." );
		KMessageBox::sorry( 0, message, i18n( "Error: Duplicate Entry" ) );
		return;
	}

	bool latOk(false), lngOk(false), tzOk(false);
	dms lat = ui->NewLat->createDms( true, &latOk );
	dms lng = ui->NewLong->createDms( true, &lngOk );
	double TZ = ui->TZBox->lineEdit()->text().toDouble( &tzOk );

	if ( ui->NewCityName->text().isEmpty() || ui->NewCountryName->text().isEmpty() ) {
		QString message = i18n( "All fields (except province) must be filled to add this location." );
		KMessageBox::sorry( 0, message, i18n( "Fields are Empty" ) );
		return;

	//FIXME after strings freeze lifts, separate TZ check from lat/long check
	} else if ( ! latOk || ! lngOk || ! tzOk ) {
		QString message = i18n( "Could not parse coordinates." );
		KMessageBox::sorry( 0, message, i18n( "Bad Coordinates" ) );
		return;
	} else {
		if ( !nameModified ) {
			QString message = i18n( "Really override original data for this city?" );
			if ( KMessageBox::questionYesNo( 0, message, i18n( "Override Existing Data?" ), i18n("Override Data"), i18n("Do Not Override")) == KMessageBox::No )
				return; //user answered No.
		}

		QString entry;
		QFile file;

		//Strip off white space
		QString name = ui->NewCityName->text().trimmed();
		QString province = ui->NewProvinceName->text().trimmed();
		QString country = ui->NewCountryName->text().trimmed();

		//check for user's city database.  If it doesn't exist, create it.
		file.setName( locateLocal( "appdata", "mycities.dat" ) ); //determine filename in local user KDE directory tree.

		if ( !file.open( QIODevice::ReadWrite | QIODevice::Append ) ) {
			QString message = i18n( "Local cities database could not be opened.\nLocation will not be recorded." );
			KMessageBox::sorry( 0, message, i18n( "Could Not Open File" ) );
			return;
		} else {
			char ltsgn = 'N'; if ( lat.degree()<0 ) ltsgn = 'S';
			char lgsgn = 'E'; if ( lng.degree()<0 ) lgsgn = 'W';
			QString TZrule = ui->DSTRuleBox->currentText();

			entry = entry.sprintf( "%-32s : %-21s : %-21s : %2d : %2d : %2d : %c : %3d : %2d : %2d : %c : %5.1f : %2s\n",
						name.toLocal8Bit().data(), province.toLocal8Bit().data(), country.toLocal8Bit().data(),
						abs(lat.degree()), lat.arcmin(), lat.arcsec(), ltsgn,
						abs(lng.degree()), lng.arcmin(), lat.arcsec(), lgsgn,
						TZ, TZrule.toLocal8Bit().data() );

			QTextStream stream( &file );
			stream << entry;
			file.close();

			//Add city to geoList...don't need to insert it alphabetically, since we always sort GeoList
			GeoLocation *g = new GeoLocation( lng.Degrees(), lat.Degrees(),
														ui->NewCityName->text(), ui->NewProvinceName->text(), ui->NewCountryName->text(),
														TZ, &p->data()->Rulebook[ TZrule ] );
			p->data()->geoList.append( g );
			
			//(possibly) insert new city into GeoBox by running filterCity()
			filterCity();
			
			//Attempt to highlight new city in list
			ui->GeoBox->setCurrentItem( 0 );
			if ( ui->GeoBox->count() ) {
				for ( uint i=0; i<ui->GeoBox->count(); i++ ) {
					if ( ui->GeoBox->item(i)->text() == g->fullName() ) {
						ui->GeoBox->setCurrentItem( i );
						break;
					}
				}
			}
		
		}
	}

	bCityAdded = true;
	return;
}

void LocationDialog::findCitiesNear( int lng, int lat ) {
	KStars *ks = (KStars *)parent();
	//find all cities within 3 degrees of (lng, lat); list them in GeoBox
	ui->GeoBox->clear();
	//Remember, do NOT delete members of filteredCityList
	while ( ! filteredCityList.isEmpty() ) filteredCityList.takeFirst();

	foreach ( GeoLocation *loc, ks->data()->geoList ) {
		if ( ( abs(	lng - int( loc->lng()->Degrees() ) ) < 3 ) &&
				 ( abs( lat - int( loc->lat()->Degrees() ) ) < 3 ) ) {
			
			ui->GeoBox->insertItem( loc->fullName() );
			filteredCityList.append( loc );
		}
	}

	ui->GeoBox->sort();
	ui->CountLabel->setText( i18np("One city matches search criteria","%n cities match search criteria", ui->GeoBox->count()) );

	if ( ui->GeoBox->firstItem() )		// set first item in list as selected
		ui->GeoBox->setCurrentItem( ui->GeoBox->firstItem() );

	repaint();
}

bool LocationDialog::checkLongLat( void ) {
	if ( ui->NewLong->text().isEmpty() || ui->NewLat->text().isEmpty() ) return false;

	bool ok(false);
	double lng = ui->NewLong->createDms(true, &ok).Degrees();
	if ( ! ok ) return false;
	double lat = ui->NewLat->createDms(true, &ok).Degrees();
	if ( ! ok ) return false;

	if ( lng < -180.0 || lng > 180.0 ) return false;
	if ( lat <  -90.0 || lat >  90.0 ) return false;

	return true;
}

void LocationDialog::clearFields( void ) {
	ui->CityFilter->clear();
	ui->ProvinceFilter->clear();
	ui->CountryFilter->clear();
	ui->NewCityName->clear();
	ui->NewProvinceName->clear();
	ui->NewCountryName->clear();
	ui->NewLong->clearFields();
	ui->NewLat->clearFields();
	ui->TZBox->lineEdit()->setText( KGlobal::locale()->formatNumber( 0.0 ) );
	ui->DSTRuleBox->setCurrentItem( 0 );
	nameModified = true;
	dataModified = false;
	ui->AddCityButton->setEnabled( false );
	ui->NewCityName->setFocus();
}

void LocationDialog::showTZRules( void ) {
	QStringList lines;
	lines.append( i18n( " Start Date (Start Time)  /  Revert Date (Revert Time)" ) );
	lines.append( " " );
	lines.append( i18n( "--: No DST correction" ) );
	lines.append( i18n( "AU: last Sun in Oct. (02:00) / last Sun in Mar. (02:00)" ) );
	lines.append( i18n( "BZ:  2nd Sun in Oct. (00:00) /  3rd Sun in Feb. (00:00)" ) );
	lines.append( i18n( "CH:  2nd Sun in Apr. (00:00) /  2nd Sun in Sep. (00:00)" ) );
	lines.append( i18n( "CL:  2nd Sun in Oct. (04:00) /  2nd Sun in Mar. (04:00)" ) );
	lines.append( i18n( "CZ:  1st Sun in Oct. (02:45) /  3rd Sun in Mar. (02:45)" ) );
	lines.append( i18n( "EE: Last Sun in Mar. (00:00) / Last Sun in Oct. (02:00)" ) );
	lines.append( i18n( "EG: Last Fri in Apr. (00:00) / Last Thu in Sep. (00:00)" ) );
	lines.append( i18n( "EU: Last Sun in Mar. (01:00) / Last Sun in Oct. (01:00)" ) );
	lines.append( i18n( "FK:  1st Sun in Sep. (02:00) /  3rd Sun in Apr. (02:00)" ) );
	lines.append( i18n( "HK:  2nd Sun in May  (03:30) /  3rd Sun in Oct. (03:30)" ) );
	lines.append( i18n( "IQ: Apr 1 (03:00) / Oct. 1 (00:00)" ) );
	lines.append( i18n( "IR: Mar 21 (00:00) / Sep. 22 (00:00)" ) );
	lines.append( i18n( "JD: Last Thu in Mar. (00:00) / Last Thu in Sep. (00:00)" ) );
	lines.append( i18n( "LB: Last Sun in Mar. (00:00) / Last Sun in Oct. (00:00)" ) );
	lines.append( i18n( "MX:  1st Sun in May  (02:00) / Last Sun in Sep. (02:00)" ) );
	lines.append( i18n( "NB:  1st Sun in Sep. (02:00) /  1st Sun in Apr. (02:00)" ) );
	lines.append( i18n( "NZ:  1st Sun in Oct. (02:00) /  3rd Sun in Mar. (02:00)" ) );
	lines.append( i18n( "PY:  1st Sun in Oct. (00:00) /  1st Sun in Mar. (00:00)" ) );
	lines.append( i18n( "RU: Last Sun in Mar. (02:00) / Last Sun in Oct. (02:00)" ) );
	lines.append( i18n( "SK:  2nd Sun in May  (00:00) /  2nd Sun in Oct. (00:00)" ) );
	lines.append( i18n( "SY: Apr. 1 (00:00) / Oct. 1 (00:00)" ) );
	lines.append( i18n( "TG:  1st Sun in Nov. (02:00) / Last Sun in Jan. (02:00)" ) );
	lines.append( i18n( "TS:  1st Sun in Oct. (02:00) / Last Sun in Mar. (02:00)" ) );
	lines.append( i18n( "US:  1st Sun in Apr. (02:00) / Last Sun in Oct. (02:00)" ) );
	lines.append( i18n( "ZN: Apr. 1 (01:00) / Oct. 1 (00:00)" ) );

	QString message = i18n( "Daylight Saving Time Rules" );
	KMessageBox::informationList( 0, message, lines, message );
}

void LocationDialog::nameChanged( void ) {
	nameModified = true;
	dataChanged();
}

//do not enable Add button until all data are present and valid.
void LocationDialog::dataChanged( void ) {
	dataModified = true;
	if ( ! ui->NewCityName->text().isEmpty() && ! ui->NewCountryName->text().isEmpty() && checkLongLat() )
		ui->AddCityButton->setEnabled( true );
	else
		ui->AddCityButton->setEnabled( false );
}

void LocationDialog::slotOk( void ) {
	bool bOkToClose = false;
	if ( addCityEnabled() ) { //user closed the location dialog without adding their new city;
		addCity();                   //call addCity() for them!
		bOkToClose = bCityAdded;
	} else {
		bOkToClose = true;
	}

	if ( bOkToClose ) accept();
}

bool LocationDialog::addCityEnabled() { return ui->AddCityButton->isEnabled(); }

#include "locationdialog.moc"
