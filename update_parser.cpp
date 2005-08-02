/*
 * Copyright (c) 2005 Christian Loose <christian.loose@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "update_parser.h"
using Cervisia::UpdateParser;


UpdateParser::UpdateParser()
    : m_simulation(false)
{
}


UpdateParser::~UpdateParser()
{
}


bool UpdateParser::isSimulation() const
{
    return m_simulation;
}


void UpdateParser::setSimulation(bool simulation)
{
    m_simulation = simulation;
}

#include "update_parser.moc"
