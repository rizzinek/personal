#include "RenderUtility\StraightProjectile.h"
#include "RenderUtility\AttackInfo.h"
#include "RenderUtility\DirectAngledRect.h"
#include "RenderUtility\LittleMap.h"
#include "Utils\MathFuncts.h"
#include "RenderUtility\RendManager.h"
#include "RenderUtility\SimpleCollisionObjs.h"

//KOSTIL'
#include "RenderUtility/CollisionRectAreaGrowing.h"

CStraightProjectile::CStraightProjectile(D3DXVECTOR3 pos, ICreature *pMaster)
: ISimpleProjectile(pos, pMaster)
, m_bNeedsKilling(false)
, m_dblAngle(0.0f)
, m_dblDistance(0.0f)
, m_dblMaxDistance(0.0f)
, m_dblVelocity(0.0f)
, m_szTexture(0)
{}

CStraightProjectile::~CStraightProjectile()
{}

DWORD CStraightProjectile::AffectEnvironment(CLevelMap *pLevel, DWORD dwTime)
{
	//draw hitbox if required
	if(CLittleRendManager::GetInstance().AreasEnabled().IsShowingDamagingAreas())
		CLittleRendManager::GetInstance().Add(m_pCollision->GetRendItem());
	//do dmg
	CHeroCreature *pTheHero = pLevel ? pLevel->GetPlayer() : 0;
	if(pTheHero && pTheHero != m_pMaster)
		if(pTheHero->HitByAttack(GetAttackInfo(), GetHitArea(), dwTime))
		{
			EndProjectileOffence();
			return 1;
		}
	CLevelMap::Creatures crt = pLevel->GetActiveCreatures();
	CLevelMap::Creatures::iterator i = crt.begin();
	while(i != crt.end())
	{
		ICreature* c = (*i);
		//if the projectiles hits a creature it disappears
		if(c != m_pMaster)
		{
			if(c->HitByAttack(GetAttackInfo(), GetHitArea(), dwTime))
			{
				EndProjectileOffence();
				return 1;
			}
		}
		++i;
	}
	if(pLevel->KickTileAsses(GetHitArea(), GetAttackInfo()))
	{
		EndProjectileOffence();
		return 1;
	}
	return 0;
}

void CStraightProjectile::EndProjectileOffence()
{
	m_bNeedsKilling = true;
	ISimpleProjectile::EndProjectileOffence();
}

IRendItem* CStraightProjectile::GetRendItem() const
{
	//return m_pCollision->GetRendItem();
	CRectAreaGrowingCollision *pCrutch = dynamic_cast<CRectAreaGrowingCollision*>(m_pCollision);
	FLOAT angle = pCrutch ? pCrutch->GetAngle() : 0.0f ;
	FLOAT len = pCrutch ? pCrutch->GetCurLength() : 1.0f;
	FLOAT width = pCrutch ? pCrutch->GetWidth() : 10.0f;
	pCrutch = 0;
	if(!m_pRendItem)
		m_pRendItem = CLittleRendManager::GetInstance().CreateQuadrilateralFromLine(0.0f, 0.0f, len * cosf(angle),
		len * sinf(angle), width, m_szTexture);
	if(m_pRendItem)
	{
		m_pRendItem->SetAreaByLine(0.0f, 0.0f, len * cosf(angle), len * sinf(angle), width);
		m_pRendItem->SetPosition(m_Position.x, m_Position.y, m_Position.z);
		return m_pRendItem;
	}
	return 0;
}

bool CStraightProjectile::NeedsKilling() const
{
	return m_bNeedsKilling;
}

DWORD CStraightProjectile::Update(DWORD dwTime)
{
	//update push/pull angle
	if(m_pAttackParameters->GetForcedMovementConfig())
		m_pAttackParameters->GetForcedMovementConfig()->SetAngle(m_dblAngle);
	FLOAT dist = m_dblVelocity * dwTime / 1000000.0f;
	if(m_dblMaxDistance > FLT_EPSILON && m_dblDistance + dist > m_dblMaxDistance)
	{
		EndProjectileOffence();
		return ERROR_SUCCESS;
	}
	ExecutionTimer.Increment(dwTime);
	if(ExecutionTimer.IsElapsed())
	{
		EndProjectileOffence();
		return ERROR_SUCCESS;
	}
	m_dblDistance += dist;
	m_Position.x += dist * cosf(m_dblAngle);
	m_Position.y += dist * sinf(m_dblAngle);
	m_pCollision->UpdateCollision(dwTime, 0);
	m_pCollision->SetPointOfOrigin(m_Position);
	return ERROR_SUCCESS;
}

FLOAT CStraightProjectile::GetAngle() const
{
	return m_dblAngle;
}

FLOAT CStraightProjectile::GetMaxDistance() const
{
	return m_dblMaxDistance;
}

FLOAT CStraightProjectile::GetVelocity() const
{
	return m_dblVelocity;
}

void CStraightProjectile::SetAngle(FLOAT dblAngle)
{
	m_dblAngle = dblAngle;
}

void CStraightProjectile::SetAngleByPoints(D3DXVECTOR3 vStart, D3DXVECTOR3 vEnd)
{
	m_dblAngle = CalculateAngle(vStart.x, vStart.y, vEnd.x, vEnd.y);
}

void CStraightProjectile::SetMaxDistance(FLOAT dist)
{
	m_dblMaxDistance = dist;
}

void CStraightProjectile::SetTexture(LPWSTR szFile)
{
	if(szFile)
	{
		m_szTexture = new wchar_t[wcsnlen_s(szFile, MAX_PATH) + 1];
		wcscpy_s(m_szTexture, wcsnlen_s(szFile, MAX_PATH) + 1, szFile);
	}
}

void CStraightProjectile::SetTimeToLive(DWORD ttl)
{
	ExecutionTimer.SetTotal(ttl);
}

void CStraightProjectile::SetVelocity(FLOAT velocity)
{
	m_dblVelocity = velocity;
}