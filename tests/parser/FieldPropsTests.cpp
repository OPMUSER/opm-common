/*
  Copyright 2019 Equinor ASA.

  This file is part of the Open Porous Media project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdexcept>
#include <iostream>
#include <memory>

#define BOOST_TEST_MODULE FieldPropsTests

#include <boost/filesystem.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

#include <opm/parser/eclipse/Parser/Parser.hpp>

#include <opm/parser/eclipse/Units/UnitSystem.hpp>
#include <opm/parser/eclipse/Deck/Section.hpp>
#include <opm/parser/eclipse/Deck/Deck.hpp>
#include <opm/parser/eclipse/Deck/DeckKeyword.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/FieldPropsManager.hpp>
#include <opm/parser/eclipse/EclipseState/Grid/EclipseGrid.hpp>


using namespace Opm;

BOOST_AUTO_TEST_CASE(CreateFieldProps) {
    EclipseGrid grid(10,10,10);
    Deck deck;
    FieldPropsManager fpm(deck, grid);
    BOOST_CHECK(!fpm.try_get<int>("SATNUM"));
    BOOST_CHECK(!fpm.try_get<double>("PORO"));
    BOOST_CHECK(!fpm.try_get<int>("SATNUM"));
    BOOST_CHECK(!fpm.try_get<double>("PORO"));
    BOOST_CHECK_THROW(fpm.get<double>("PORO"), std::out_of_range);
    BOOST_CHECK_THROW(fpm.get<int>("SATNUM"), std::out_of_range);
    BOOST_CHECK_THROW(fpm.get_global<double>("PORO"), std::out_of_range);
    BOOST_CHECK_THROW(fpm.get_global<int>("SATNUM"), std::out_of_range);
    BOOST_CHECK_THROW(fpm.try_get<int>("NOT_SUPPORTED"), std::invalid_argument);
    BOOST_CHECK_THROW(fpm.try_get<double>("NOT_SUPPORTED"), std::invalid_argument);
    BOOST_CHECK_THROW(fpm.get<int>("NOT_SUPPORTED"), std::invalid_argument);
    BOOST_CHECK_THROW(fpm.get<double>("NOT_SUPPORTED"), std::invalid_argument);

    BOOST_CHECK_THROW(fpm.get_global<double>("NO1"), std::invalid_argument);
    BOOST_CHECK_THROW(fpm.get_global<int>("NO2"), std::invalid_argument);
}



BOOST_AUTO_TEST_CASE(CreateFieldProps2) {
    std::string deck_string = R"(
GRID

PORO
   1000*0.10 /
)";
    std::vector<int> actnum(1000, 1);
    for (std::size_t i=0; i< 1000; i += 2)
        actnum[i] = 0;
    EclipseGrid grid(EclipseGrid(10,10,10), actnum);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, grid);


    const auto& poro1 = fpm.get<double>("PORO");
    BOOST_CHECK_EQUAL(poro1.size(), grid.getNumActive());

    const auto& poro2 = fpm.try_get<double>("PORO");
    BOOST_CHECK(poro1 == *poro2);
}


BOOST_AUTO_TEST_CASE(INVALID_COPY) {
    std::string deck_string = R"(
GRID

COPY
   PERMX PERMY /
/
)";

    EclipseGrid grid(EclipseGrid(10,10,10));
    Deck deck = Parser{}.parseString(deck_string);
    BOOST_CHECK_THROW( FieldPropsManager(deck, grid), std::invalid_argument);
}

BOOST_AUTO_TEST_CASE(GRID_RESET) {
    std::string deck_string = R"(
REGIONS

SATNUM
0 1 2 3 4 5 6 7 8
/
)";
    std::vector<int> actnum1 = {1,1,1,0,0,0,1,1,1};
    EclipseGrid grid(3,1,3); grid.resetACTNUM(actnum1);
    Deck deck = Parser{}.parseString(deck_string);
    FieldPropsManager fpm(deck, grid);
    const auto& s1 = fpm.get<int>("SATNUM");
    BOOST_CHECK_EQUAL(s1.size(), 6);
    BOOST_CHECK_EQUAL(s1[0], 0);
    BOOST_CHECK_EQUAL(s1[1], 1);
    BOOST_CHECK_EQUAL(s1[2], 2);
    BOOST_CHECK_EQUAL(s1[3], 6);
    BOOST_CHECK_EQUAL(s1[4], 7);
    BOOST_CHECK_EQUAL(s1[5], 8);

    std::vector<int> actnum2 = {1,0,1,0,0,0,1,0,1};
    grid.resetACTNUM(actnum2);
    fpm.reset_grid(grid);

    BOOST_CHECK_EQUAL(s1.size(), 4);
    BOOST_CHECK_EQUAL(s1[0], 0);
    BOOST_CHECK_EQUAL(s1[1], 2);
    BOOST_CHECK_EQUAL(s1[2], 6);
    BOOST_CHECK_EQUAL(s1[3], 8);

    grid.resetACTNUM(actnum1);
    BOOST_CHECK_THROW(fpm.reset_grid(grid), std::logic_error);
}